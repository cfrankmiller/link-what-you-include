// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <util/byte_visitor.hpp>
#include <util/byte_visitor/cstdint.hpp>

#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>

namespace util
{
template <typename T>
  requires(std::is_convertible_v<T, std::string_view>)
void visit(Byte_sink_visitor auto& v, T&& value)
{
  const std::string_view view(std::forward<T>(value));
  const auto length = static_cast<uint64_t>(view.size());
  v(length);
  if (0U < length)
  {
    v(std::as_bytes(std::span(view.data(), view.size())));
  }
}

inline void visit(Byte_source_visitor auto& v, std::string& value)
{
  uint64_t length = 0;
  v(length);
  if (!v)
  {
    return;
  }

  if (!v.can_read(static_cast<size_t>(length)))
  {
    v.set_error();
    return;
  }

  if (0 < length)
  {
    std::string str(static_cast<size_t>(length), '\0');
    v(std::as_writable_bytes(std::span(str.data(), str.size())));
    value.swap(str);
  }
}
} // namespace util
