// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#include <src/real_file_loader.hpp>

#include <fmt/format.h>
#include <simdjson.h>
#include <tl/expected.hpp>

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <ios>
#include <string>

namespace target_model
{
auto Real_file_loader::load(const std::filesystem::path& path)
  -> tl::expected<void, std::string>
{
  std::ifstream ifs(path.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
  if (ifs.fail())
  {
    return tl::unexpected(fmt::format("Failed to open {}", path.string()));
  }

  size_ = ifs.tellg();
  ifs.seekg(0, std::ios::beg);
  bytes_.resize(size_ + simdjson::SIMDJSON_PADDING);

  ifs.read(bytes_.data(), static_cast<std::streamsize>(size_));
  if (ifs.fail())
  {
    return tl::unexpected(fmt::format("Failed to read {}", path.string()));
  }

  return {};
}

auto Real_file_loader::data() const -> const char*
{
  return bytes_.data();
}

auto Real_file_loader::size() const -> size_t
{
  return size_;
}

auto Real_file_loader::size_with_padding() const -> size_t
{
  return bytes_.size();
}
} // namespace target_model
