// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <util/hash.hpp>

#include <expected>
#include <filesystem>
#include <memory>
#include <span>
#include <vector>

namespace util
{
class Key_value_store
{
public:
  virtual ~Key_value_store() = default;
  [[nodiscard]] virtual std::expected<std::vector<std::byte>, std::string> read(Hash_value key) const = 0;
  virtual std::expected<void, std::string> write(Hash_value key, std::span<const std::byte> value) = 0;
};

std::unique_ptr<Key_value_store> default_key_value_store(std::filesystem::path cache_dir);
} // namespace util
