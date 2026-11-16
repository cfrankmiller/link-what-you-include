// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#include <src/scan_impl.hpp>

#include <message/message.hpp>
#include <scanner/include.hpp>
#include <src/compile_command.hpp>
#include <src/include_sets.hpp>
#include <target_model/target_data.hpp>
#include <util/byte_hasher.hpp>
#include <util/hash.hpp>

#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Basic/FileEntry.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/FileSystemOptions.h>
#include <clang/Basic/LLVM.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Lex/PPCallbacks.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Lex/PreprocessorOptions.h>
#include <clang/Serialization/PCHContainerOperations.h>
#include <clang/Tooling/DependencyScanning/DependencyScanningFilesystem.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/IntrusiveRefCntPtr.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Support/ErrorOr.h>
#include <llvm/Support/VirtualFileSystem.h>

#include <cassert>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <format>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace clang::dependency_directives_scan
{
struct Directive;
} // namespace clang::dependency_directives_scan

namespace scanner
{
std::filesystem::path to_normal_path(const std::string& path)
{
  return std::filesystem::path(path).lexically_normal().generic_string();
}

enum class Context : uint8_t
{
  arbitrary_file,
  source_file,
  interface_header
};

std::string_view to_string(Context context)
{
  switch (context)
  {
    case Context::interface_header:
      return "interface header";
    case Context::source_file:
      return "source";
    case Context::arbitrary_file:
      return "arbitrary file";
  }
  std::unreachable();
}

struct Internal_data
{
  Include_sets include_sets;
  std::vector<std::filesystem::path> transitive_includes;
  util::Byte_hasher transitive_include_hasher;
};

class PPRecorder : public clang::PPCallbacks
{
  const clang::Preprocessor& preprocessor_;
  Internal_data& internal_data_;
  const target_model::Target_data& target_data_;

  Source_line last_include_loc_;
  std::vector<Source_line> include_chain_;
  std::filesystem::path main_source_file_;
  std::filesystem::path current_source_file_;
  Context context_{Context::arbitrary_file};
  int external_depth_ = 0;
  bool in_private_ = false;
  bool in_system_ = false;

  std::map<std::filesystem::path, Include_set> interface_header_includes_;

  std::set<std::filesystem::path> transitive_include_set_;

public:
  PPRecorder(const clang::Preprocessor& preprocessor,
             Internal_data& internal_data,
             const target_model::Target_data& target_data)
  : preprocessor_(preprocessor),
    internal_data_(internal_data),
    target_data_(target_data)
  {
    const auto main_fid = preprocessor_.getSourceManager().getMainFileID();
    if (auto main_file_entry_ref =
          preprocessor_.getSourceManager().getFileEntryRefForID(main_fid);
        main_file_entry_ref.has_value())
    {
      main_source_file_ = to_normal_path(main_file_entry_ref->getNameAsRequested().str());
      in_private_ = target_model::is_private_source(target_data_, main_source_file_);
      message::debug("main file {} is {} private source file",
                     main_source_file_.string(),
                     in_private_ ? "a" : "not a");
    }
    else
    {
      // TODO
      assert(!"is this code path possible?");
    }
  }

  ~PPRecorder() override = default;
  PPRecorder(const PPRecorder&) = delete;
  PPRecorder(PPRecorder&&) noexcept = delete;
  PPRecorder& operator=(const PPRecorder&) = delete;
  PPRecorder& operator=(PPRecorder&&) noexcept = delete;

  void LexedFileChanged(clang::FileID fid,
                        LexedFileChangeReason reason,
                        clang::SrcMgr::CharacteristicKind file_type,
                        clang::FileID prev_fid,
                        clang::SourceLocation /*loc*/) override
  {
    // ignore predefines file
    if ((reason == LexedFileChangeReason::EnterFile &&
         fid == preprocessor_.getPredefinesFileID()) ||
        (reason == LexedFileChangeReason::ExitFile &&
         prev_fid == preprocessor_.getPredefinesFileID()))
    {
      return;
    }

    // current file
    const auto previous_source_file = current_source_file_;
    if (auto file_entry_ref = preprocessor_.getSourceManager().getFileEntryRefForID(fid);
        file_entry_ref.has_value())
    {
      current_source_file_ = to_normal_path(file_entry_ref->getNameAsRequested().str());
    }
    else
    {
      assert(!"We expect to always get a valid file entry ref");
      context_ = Context::arbitrary_file;
      return;
    }

    // context
    const auto previous_context = context_;
    const bool previous_in_system = in_system_;
    in_system_ = clang::SrcMgr::isSystem(file_type);
    const bool in_main_file = (fid == preprocessor_.getSourceManager().getMainFileID());
    if (!in_main_file && target_model::is_interface_header(target_data_, current_source_file_))
    {
      context_ = Context::interface_header;
    }
    else if (target_model::is_private_source(target_data_, current_source_file_))
    {
      context_ = Context::source_file;
    }
    else
    {
      context_ = Context::arbitrary_file;
    }

    if (reason == LexedFileChangeReason::EnterFile)
    {
      if (!previous_in_system || !in_system_)
      {
        message::debug("Enter {} {} ({})",
                       to_string(context_),
                       current_source_file_.string(),
                       fid.getHashValue());
      }

      if (!in_main_file && !in_system_)
      {
        auto relative_path =
          current_source_file_.lexically_relative(main_source_file_.parent_path());
        if (transitive_include_set_.insert(relative_path).second)
        {
          internal_data_.transitive_includes.push_back(relative_path);
          std::optional<llvm::MemoryBufferRef> buffRef =
            preprocessor_.getSourceManager().getBufferOrNone(fid);
          assert(buffRef.has_value());

          // Note: we are intentionally hashing hash values so we can cache file hashes when doing lookups. There should
          // be sufficient collision prevention because we are using a 128-bit hash.
          const auto file_hash = util::hash(
            std::string_view(buffRef->getBufferStart(), buffRef->getBufferSize()));
          internal_data_.transitive_include_hasher(file_hash);
        }
      }

      if (!last_include_loc_.source.empty())
      {
        include_chain_.emplace_back(last_include_loc_);
      }

      if (context_ == Context::interface_header)
      {
        interface_header_includes_.try_emplace(current_source_file_);
      }

      if (previous_context != Context::arbitrary_file && context_ == Context::arbitrary_file)
      {
        if (in_private_ && external_depth_ == 0)
        {
          message::debug("Private dependency added: {}", current_source_file_.string());
          internal_data_.include_sets.includes.emplace(Include{current_source_file_, include_chain_});
        }

        if (previous_context == Context::interface_header)
        {
          message::debug("Interface dependency added: {}", current_source_file_.string());
          internal_data_.include_sets.interface_includes.emplace(
            Include{current_source_file_, {last_include_loc_}});
        }
      }

      if (!in_main_file && context_ == Context::arbitrary_file)
      {
        ++external_depth_;

        if (auto it = interface_header_includes_.find(previous_source_file);
            it != interface_header_includes_.end())
        {
          it->second.emplace(Include{current_source_file_, {last_include_loc_}});
        }
      }
    }
    else
    {
      if (!previous_in_system || !in_system_)
      {
        message::debug("Reenter {} {} ({})",
                       to_string(context_),
                       current_source_file_.string(),
                       fid.getHashValue());
      }

      if (previous_context == Context::arbitrary_file)
      {
        assert(0 < external_depth_);
        --external_depth_;
      }

      assert(!include_chain_.empty());

      if (previous_context == Context::interface_header &&
          context_ == Context::interface_header)
      {
        message::debug("Interface dependency propagation");

        if (auto pit = interface_header_includes_.find(previous_source_file),
            it = interface_header_includes_.find(current_source_file_);
            pit != interface_header_includes_.end() && it != interface_header_includes_.end())
        {
          for (const Include& include : pit->second)
          {
            std::vector<Source_line> include_chain;
            include_chain.push_back(include_chain_.back());
            include_chain.insert(include_chain.end(),
                                 include.include_chain.begin(),
                                 include.include_chain.end());
            it->second.emplace(Include{include.path, include_chain});
          }
        }
      }

      include_chain_.pop_back();
    }
  }

  void InclusionDirective(clang::SourceLocation include_loc,
                          const clang::Token& /*token*/,
                          clang::StringRef spelled_filename,
                          bool is_angled,
                          clang::CharSourceRange /*filename_range*/,
                          clang::OptionalFileEntryRef /*file*/,
                          clang::StringRef /*searchPath*/,
                          clang::StringRef /*relativePath*/,
                          const clang::Module* /*imported*/,
                          clang::SrcMgr::CharacteristicKind /*file_type*/) override
  {
    auto presumed_loc = preprocessor_.getSourceManager().getPresumedLoc(include_loc);
    assert(presumed_loc.isValid());
    assert(current_source_file_ == to_normal_path(presumed_loc.getFilename()));
    last_include_loc_ = Source_line{current_source_file_, presumed_loc.getLine()};

    if (!in_system_)
    {
      message::debug("Include {}{}{} from {}:{}",
                     is_angled ? '<' : '"',
                     spelled_filename.str(),
                     is_angled ? '>' : '"',
                     last_include_loc_.source.string(),
                     last_include_loc_.line);
    }
  }

  void FileSkipped(const clang::FileEntryRef& file,
                   const clang::Token& /*filename_tok*/,
                   clang::SrcMgr::CharacteristicKind /*file_type*/) override
  {
    if (context_ == Context::arbitrary_file)
    {
      return;
    }

    const auto filename = to_normal_path(file.getNameAsRequested().str());

    message::debug("File skipped: {}", filename.string());

    Context context = Context::arbitrary_file;
    if (target_model::is_interface_header(target_data_, filename))
    {
      context = Context::interface_header;
    }
    else if (target_model::is_private_source(target_data_, filename))
    {
      context = Context::source_file;
    }

    if (in_private_ && context == Context::interface_header)
    {
      assert(interface_header_includes_.contains(filename));
      if (auto it = interface_header_includes_.find(filename);
          it != interface_header_includes_.end())
      {
        message::debug("Dependency propagation");
        for (const auto& e : it->second)
        {
          message::debug("Private dependency added: {}", e.path.string());
          auto include_chain = include_chain_;
          include_chain.emplace_back(last_include_loc_);
          include_chain.insert(include_chain.end(),
                               e.include_chain.begin(),
                               e.include_chain.end());
          internal_data_.include_sets.includes.emplace(Include{e.path, include_chain});
        }
      }
    }
    else if (context == Context::arbitrary_file)
    {
      if (in_private_)
      {
        assert(!last_include_loc_.source.empty());
        message::debug("Private dependency added: {}", filename.string());
        auto include_chain = include_chain_;
        include_chain.emplace_back(last_include_loc_);
        internal_data_.include_sets.includes.emplace(Include{filename, include_chain});
      }

      if (context_ == Context::interface_header)
      {
        assert(!last_include_loc_.source.empty());
        message::debug("Interface dependency added: {}", filename.string());
        internal_data_.include_sets.interface_includes.emplace(Include{filename, {last_include_loc_}});
      }
    }
  }
};

class Action : public clang::PreprocessOnlyAction
{
  Internal_data& internal_data_;
  const target_model::Target_data& target_data_;

  llvm::IntrusiveRefCntPtr<clang::tooling::dependencies::DependencyScanningWorkerFilesystem> dep_fs_;

public:
  Action(Internal_data& internal_data,
         const target_model::Target_data& target_data,
         llvm::IntrusiveRefCntPtr<clang::tooling::dependencies::DependencyScanningWorkerFilesystem> dep_fs)
  : internal_data_(internal_data),
    target_data_(target_data),
    dep_fs_(std::move(dep_fs))
  {
  }

private:
  void ExecuteAction() override
  {
    auto& compiler_instance = getCompilerInstance();

    compiler_instance.getDiagnosticOpts().IgnoreWarnings = true;
    compiler_instance.getDiagnostics().setIgnoreAllWarnings(true);

    compiler_instance.getPreprocessorOpts().DependencyDirectivesForFile =
      [dep_fs = dep_fs_](clang::FileEntryRef file)
      -> std::optional<llvm::ArrayRef<clang::dependency_directives_scan::Directive>>
    {
      if (llvm::ErrorOr<clang::tooling::dependencies::EntryRef> entry =
            dep_fs->getOrCreateFileSystemEntry(file.getNameAsRequested()))
      {
        return entry->getDirectiveTokens();
      }
      return std::nullopt;
    };

    auto& preprocessor = compiler_instance.getPreprocessor();
    preprocessor.addPPCallbacks(
      std::make_unique<PPRecorder>(preprocessor, internal_data_, target_data_));

    PreprocessOnlyAction::ExecuteAction();
  }
};

class Action_factory : public clang::tooling::FrontendActionFactory
{
public:
  Action_factory(Internal_data& internal_data,
                 const target_model::Target_data& target_data,
                 llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem> file_system,
                 clang::tooling::dependencies::DependencyScanningFilesystemSharedCache& dep_cache)
  : internal_data_(internal_data),
    target_data_(target_data),
    file_system_(std::move(file_system)),
    dep_cache_(dep_cache)
  {
  }

  ~Action_factory() override = default;
  Action_factory(const Action_factory&) = delete;
  Action_factory(Action_factory&&) noexcept = delete;
  Action_factory& operator=(const Action_factory&) = delete;
  Action_factory& operator=(Action_factory&&) noexcept = delete;

  std::unique_ptr<clang::FrontendAction> create() override
  {
    auto dep_fs =
      llvm::IntrusiveRefCntPtr<clang::tooling::dependencies::DependencyScanningWorkerFilesystem>{
        new clang::tooling::dependencies::DependencyScanningWorkerFilesystem(dep_cache_,
                                                                             file_system_)};
    return std::make_unique<Action>(internal_data_, target_data_, std::move(dep_fs));
  }

private:
  Internal_data& internal_data_;
  const target_model::Target_data& target_data_;
  llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem> file_system_;
  clang::tooling::dependencies::DependencyScanningFilesystemSharedCache& dep_cache_;
};

std::expected<Include_data, std::string> scan_impl(
  const llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem>& file_system,
  clang::tooling::dependencies::DependencyScanningFilesystemSharedCache& dep_cache,
  const target_model::Target_data& target_data,
  const Compile_command& compile_command)
{
  if (file_system->setCurrentWorkingDirectory(compile_command.cwd.string()))
  {
    return std::unexpected(std::format("Cannot chdir into {}", compile_command.cwd.string()));
  }

  Internal_data internal_data;
  Action_factory action_factory(internal_data, target_data, file_system, dep_cache);
  auto file_manager = llvm::IntrusiveRefCntPtr<clang::FileManager>{
    new clang::FileManager(clang::FileSystemOptions(), file_system)};
  auto pch_container_ops = std::make_shared<clang::PCHContainerOperations>();

  clang::tooling::ToolInvocation invocation(compile_command.command,
                                            &action_factory,
                                            file_manager.get(),
                                            pch_container_ops);
  if (!invocation.run())
  {
    return std::unexpected(
      std::format("Error while processing {}.\n", compile_command.source.string()));
  }

  return Include_data{.include_sets = std::move(internal_data.include_sets),
                      .transitive_includes = std::move(internal_data.transitive_includes),
                      .transitive_includes_hash =
                        internal_data.transitive_include_hasher.digest()};
}
} // namespace scanner
