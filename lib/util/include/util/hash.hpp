// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <span>
#include <string_view>
#include <array>

namespace util
{
using Hash_value = std::array<std::byte, 16>;

Hash_value hash(std::span<const std::byte> bytes);
Hash_value hash(std::string_view bytes);
Hash_value hash(const void* data, size_t size);
} // namespace util
