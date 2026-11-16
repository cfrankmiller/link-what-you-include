// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <span>

namespace util
{
template <typename T>
concept Byte_source_visitor = requires(T obj, std::span<std::byte> span, size_t size) {
    obj(span);
    obj.set_error();
    { obj.can_read(size) } -> std::convertible_to<bool>;
    { T::is_source } -> std::convertible_to<bool>;
} && (T::is_source == true);

template <typename T>
concept Byte_sink_visitor = requires(T obj, std::span<const std::byte> s) {
    obj(s);
  { T::is_source } -> std::convertible_to<bool>;
} && (T::is_source == false);

template <typename V>
concept Byte_visitor = Byte_source_visitor<V> || Byte_sink_visitor<V>;
} // namespace util
