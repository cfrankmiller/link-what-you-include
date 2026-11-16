// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <util/hash.hpp>

#include <filesystem>
#include <map>
#include <mutex>

namespace util
{
class Hashed_file_view
{
public:
  Hashed_file_view() = default;
  ~Hashed_file_view() = default;
  Hashed_file_view(const Hashed_file_view& other) = delete;
  Hashed_file_view(Hashed_file_view&& other) noexcept = delete;
  Hashed_file_view& operator=(const Hashed_file_view& other) = delete;
  Hashed_file_view& operator=(Hashed_file_view&& other) noexcept = delete;

  [[nodiscard]] std::optional<Hash_value> hash_of(const std::filesystem::path& file) const;

private:
  struct Entry
  {
    std::filesystem::file_time_type mtime{std::filesystem::file_time_type::min()};
    util::Hash_value hash_value{};
  };
  mutable std::mutex cache_mutex_;
  mutable std::map<std::filesystem::path, Entry> cache_;
};
} // namespace util
