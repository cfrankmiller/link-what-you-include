// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#include <util/hashed_file_view.hpp>

#include <fstream>
#include <vector>

namespace util
{
std::optional<Hash_value> Hashed_file_view::hash_of(const std::filesystem::path& file) const
{
  std::error_code ec;
  auto mtime = std::filesystem::last_write_time(file, ec);
  if (ec || !std::filesystem::is_regular_file(file))
  {
    const std::lock_guard lock(cache_mutex_);
    cache_.erase(file);
    return std::nullopt;
  }

  {
    const std::lock_guard lock(cache_mutex_);
    auto& entry = cache_[file];
    if (mtime <= entry.mtime)
    {
      return entry.hash_value;
    }
  }

  std::ifstream ifs(file, std::ios::binary | std::ios::ate);
  if (ifs.fail())
  {
    return std::nullopt;
  }

  auto size = ifs.tellg();
  ifs.seekg(0, std::ios::beg);
  std::vector<char> buffer; // TODO: consider using thread_local?
  buffer.resize(size);

  ifs.read(buffer.data(), static_cast<std::streamsize>(size));
  if (ifs.fail())
  {
    return std::nullopt;
  }

  const auto hash_value = util::hash(buffer.data(), buffer.size());

  const std::lock_guard lock(cache_mutex_);
  auto& entry = cache_[file];
  if (mtime <= entry.mtime)
  {
    return entry.hash_value;
  }

  entry.mtime = mtime;
  entry.hash_value = hash_value;

  return hash_value;
}
} // namespace util
