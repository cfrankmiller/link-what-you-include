// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#include <target_model/target_data.hpp>

#include <util/utils.hpp>

#include <filesystem>

namespace target_model
{
auto is_interface_header(const Target_data& target_data,
                         const std::filesystem::path& filename) -> bool
{
  if (auto it = target_data.interface_headers.find(filename);
      it != target_data.interface_headers.end())
  {
    return true;
  }

  for (const auto& directory : target_data.interface_include_directories)
  {
    if (target_data.interface_include_prefixes.empty())
    {
      if (util::is_in_directory(directory, filename))
      {
        return true;
      }
    }
    else
    {
      for (const auto& prefix : target_data.interface_include_prefixes)
      {
        if (util::is_in_directory(directory / prefix, filename))
        {
          return true;
        }
      }
    }
  }

  return false;
}

auto is_private_source(const Target_data& target_data,
                       const std::filesystem::path& filename) -> bool
{
  auto it = target_data.sources.find(filename);
  return it != target_data.sources.end();
}
} // namespace target_model
