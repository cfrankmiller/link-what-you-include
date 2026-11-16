// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <util/hash.hpp>

#include <memory>
#include <span>

namespace util
{
class Byte_hasher
{
public:
  Byte_hasher();
  ~Byte_hasher();
  Byte_hasher(const Byte_hasher& other);
  Byte_hasher(Byte_hasher&& other) noexcept;
  Byte_hasher& operator=(const Byte_hasher& other);
  Byte_hasher& operator=(Byte_hasher&& other) noexcept;

  void reset();
  [[nodiscard]] Hash_value digest() const;

  friend void swap(Byte_hasher& lhs, Byte_hasher& rhs) noexcept
  {
    std::swap(lhs.impl_, rhs.impl_);
  }

  // byte_sink interface

  explicit operator bool() const
  {
    return true;
  }

  static constexpr bool is_source = false;

  void operator()(std::span<const std::byte> bytes);

  template <typename T>
    requires(!std::is_convertible_v<std::add_lvalue_reference_t<T>, std::span<const std::byte>>)
  void operator()(const T& val)
  {
    visit(*this, val);
  }

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};
} // namespace util
