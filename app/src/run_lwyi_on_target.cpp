// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#include <src/run_lwyi_on_target.hpp>

#include <lwyi/check_target.hpp>
#include <lwyi/dependency_visibility.hpp>
#include <scanner/include.hpp>
#include <scanner/scan.hpp>
#include <target_model/target.hpp>
#include <target_model/target_data.hpp>

#include <expected>
#include <filesystem>
#include <print>
#include <unordered_set>
#include <vector>

auto run_lwyi_on_target(const target_model::Target_model& target_model,
                        const std::filesystem::path& binary_dir,
                        const target_model::Target& target,
                        const target_model::Target_data& target_data,
                        unsigned int num_threads) -> bool
{
  static scanner::Scanner scanner(num_threads);
  if (target_data.sources.empty() && target_data.verify_interface_header_sets_sources.empty())
  {
    std::print("No sources. Skipping.\n");
    return true;
  }

  auto eincludes = scanner.scan(binary_dir, target_data);
  if (!eincludes.has_value())
  {
    std::print("error: Failed to scan the direct includes of target {}\n{}\n",
               target.name,
               eincludes.error());
    return false;
  }

  auto errors = lwyi::check_target(target_model, target, target_data, *eincludes);

  // TODO: consider enabling the following with a command line option
#if 0
  // special case: ignore linked PUBLIC but included INTERFACE errors
  errors.erase(std::remove_if(errors.begin(),
                              errors.end(),
                              [](const lwyi::LWYI_error& error)
                              {
                                return error.linked_visibility ==
                                         lwyi::Dependency_visibility::public_scope &&
                                       error.included_visibility ==
                                         lwyi::Dependency_visibility::interface_scope;
                              }),
               errors.end());
#endif

  if (errors.empty())
  {
    return true;
  }

  for (const auto& error : errors)
  {
    std::print("error: {} ", target.name);
    switch (error.linked_visibility)
    {
      case lwyi::Dependency_visibility::none:
        std::print("does not link to {} ", error.target.name);
        break;
      case lwyi::Dependency_visibility::private_scope:
        std::print("links to {} with PRIVATE scope ", error.target.name);
        break;
      case lwyi::Dependency_visibility::interface_scope:
        std::print("links to {} with INTERFACE scope ", error.target.name);
        break;
      case lwyi::Dependency_visibility::public_scope:
        std::print("links to {} with PUBLIC scope ", error.target.name);
        break;
    }
    std::print("but it is ");
    switch (error.included_visibility)
    {
      case lwyi::Dependency_visibility::none:
        std::print("not included.\n");
        break;
      case lwyi::Dependency_visibility::private_scope:
        std::print("included with PRIVATE scope.\n");
        break;
      case lwyi::Dependency_visibility::interface_scope:
        std::print("included with INTERFACE scope.\n");
        break;
      case lwyi::Dependency_visibility::public_scope:
        std::print("included with PUBLIC scope.\n");
        break;
    }

    for (const auto& include : error.sample_includes)
    {
      std::print("note: {}\n", include.path.string());
      for (auto it = include.include_chain.rbegin(); it != include.include_chain.rend(); ++it)
      {
        const auto& source_line = *it;
        std::print("  included from {}:{}\n", source_line.source.string(), source_line.line);
      }
    }
  }

  return false;
}
