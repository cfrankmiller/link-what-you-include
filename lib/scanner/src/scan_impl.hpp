// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <scanner/include.hpp>
#include <scanner/scan.hpp>
#include <src/include_sets.hpp>
#include <util/hash.hpp>

#include <expected>
#include <string>

namespace llvm
{
template <typename T>
class IntrusiveRefCntPtr;
}

namespace llvm::vfs
{
class FileSystem;
}

namespace clang::tooling::dependencies
{
class DependencyScanningFilesystemSharedCache;
}

namespace target_model
{
struct Target_data;
}

namespace scanner
{
struct Compile_command;

struct Include_data
{
  Include_sets include_sets;
  std::vector<std::filesystem::path> transitive_includes;
  util::Hash_value transitive_includes_hash;
};

std::expected<Include_data, std::string> scan_impl(
  const llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem>& file_system,
  clang::tooling::dependencies::DependencyScanningFilesystemSharedCache& dep_cache,
  const target_model::Target_data& target_data,
  const Compile_command& compile_command);
} // namespace scanner
