// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstddef>
#include <span>
#include <vector>

namespace util
{
class Byte_writer
{
public:
  explicit operator bool() const
  {
    return true;
  }

  [[nodiscard]] std::span<const std::byte> view_buffer() const
  {
    return {buffer_.data(), buffer_.size()};
  }

  void clear()
  {
    buffer_.clear();
  }

  void reserve(size_t capacity)
  {
    buffer_.reserve(capacity);
  }

  // byte_sink interface

  static constexpr bool is_source = false;

  void operator()(std::span<const std::byte> bytes)
  {
    buffer_.insert_range(buffer_.end(), bytes);
  }

  template <typename T>
    requires(!std::is_convertible_v<std::add_lvalue_reference_t<T>, std::span<const std::byte>>)
  void operator()(const T& val)
  {
    visit(*this, val);
  }

private:
  std::vector<std::byte> buffer_;
};
} // namespace util

