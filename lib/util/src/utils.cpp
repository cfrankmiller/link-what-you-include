// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#include <util/utils.hpp>

#include <filesystem>

namespace util
{
auto is_in_directory(std::filesystem::path dir, const std::filesystem::path& file)
  -> bool
{
  auto relative_path = file.lexically_relative(dir);
  return !relative_path.empty() && *relative_path.begin() != "..";
}
} // namespace util
