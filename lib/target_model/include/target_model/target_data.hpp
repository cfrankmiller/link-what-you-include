// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <target_model/target.hpp>

#include <cstddef>
#include <filesystem>
#include <string>
#include <unordered_set>

namespace target_model
{
struct Path_hash
{
  using argument_type = std::filesystem::path;
  using result_type = std::size_t;

  auto operator()(const std::filesystem::path& p) const noexcept -> std::size_t
  {
    return std::filesystem::hash_value(p);
  }
};

struct Target_data
{
  std::unordered_set<std::filesystem::path, Path_hash> interface_headers;
  std::unordered_set<std::filesystem::path, Path_hash> interface_include_directories;
  std::unordered_set<std::string> interface_include_prefixes;
  std::unordered_set<Target> interface_dependencies;
  std::unordered_set<Target> dependencies;
  std::unordered_set<std::filesystem::path, Path_hash> sources;
  std::unordered_set<std::filesystem::path, Path_hash> verify_interface_header_sets_sources;
};

auto is_interface_header(const Target_data& target_data,
                         const std::filesystem::path& filename) -> bool;
auto is_private_source(const Target_data& target_data,
                       const std::filesystem::path& filename) -> bool;
} // namespace target_model
