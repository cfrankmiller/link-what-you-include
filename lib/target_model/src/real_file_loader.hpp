// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <src/file_loader.hpp>

#include <cstddef>
#include <string>
#include <vector>

namespace target_model
{
class Real_file_loader : public File_loader
{
public:
  [[nodiscard]] auto load(const std::filesystem::path& path)
    -> tl::expected<void, std::string> override;
  [[nodiscard]] auto data() const -> const char* override;
  [[nodiscard]] auto size() const -> size_t override;
  [[nodiscard]] auto size_with_padding() const -> size_t override;

private:
  size_t size_{0U};
  std::vector<char> bytes_;
};
} // namespace target_model
