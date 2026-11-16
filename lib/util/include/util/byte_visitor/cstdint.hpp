// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <util/byte_visitor.hpp>

#include <bit>
#include <type_traits>

namespace util
{
template <typename T>
  requires(std::is_integral_v<T>)
void visit(Byte_sink_visitor auto& v, T value)
{
  if constexpr (std::endian::native == std::endian::big)
  {
    value = std::byteswap(value);
  }
  v(std::as_bytes(std::span(&value, 1)));
}

template <typename T>
  requires(std::is_integral_v<T>)
void visit(Byte_source_visitor auto& v, T& value)
{
  v(std::as_writable_bytes(std::span(&value, 1)));

  if constexpr (std::endian::native == std::endian::big)
  {
    value = std::byteswap(value);
  }
}
} // namespace util
