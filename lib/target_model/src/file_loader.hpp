// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <tl/expected.hpp>

#include <filesystem>

namespace target_model
{
class File_loader
{
public:
  virtual ~File_loader() = default;
  [[nodiscard]] virtual auto load(const std::filesystem::path& path)
    -> tl::expected<void, std::string> = 0;
  [[nodiscard]] virtual auto data() const -> const char* = 0;
  [[nodiscard]] virtual auto size() const -> size_t = 0;
  [[nodiscard]] virtual auto size_with_padding() const -> size_t = 0;
};
} // namespace target_model
