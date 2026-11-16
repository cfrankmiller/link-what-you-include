// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#include <util/hash.hpp>

#define XXH_STATIC_LINKING_ONLY
#include <xxhash.h>

#include <cassert>
#include <string_view>

namespace util
{
Hash_value hash(std::span<const std::byte> bytes)
{
  return std::bit_cast<Hash_value>(XXH3_128bits(bytes.data(), bytes.size()));
}

Hash_value hash(std::string_view bytes)
{
  return std::bit_cast<Hash_value>(XXH3_128bits(bytes.data(), bytes.size()));
}

Hash_value hash(const void* data, size_t size)
{
  return std::bit_cast<Hash_value>(XXH3_128bits(data, size));
}
} // namespace util
