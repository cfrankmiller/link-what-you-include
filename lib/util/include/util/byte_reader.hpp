// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstring>
#include <span>

namespace util
{
class Byte_reader
{
public:
  explicit Byte_reader(std::span<const std::byte> span)
  : span_(span)
  {
  }

  explicit operator bool() const
  {
    return good_;
  }

  // byte_source interface

  static constexpr bool is_source = true;

  [[nodiscard]] bool can_read(size_t byte_count) const
  {
    return byte_count <= span_.size();
  }

  void set_error()
  {
    good_ = false;
  }

  void operator()(std::span<std::byte> bytes)
  {
    if (!good_)
    {
      return;
    }

    if (span_.size() < bytes.size())
    {
      good_ = false;
      return;
    }

    std::memcpy(bytes.data(), span_.data(), bytes.size());
    span_ = span_.subspan(bytes.size());
  }

  template <typename T>
    requires(!std::is_convertible_v<std::add_lvalue_reference_t<T>, std::span<std::byte>>)
  void operator()(T& val)
  {
    if (good_)
    {
      visit(*this, val);
    }
  }

private:
  std::span<const std::byte> span_;
  bool good_ = true;
};
} // namespace util


