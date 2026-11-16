// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <utility>

namespace util
{
template <typename F>
class scope_exit
{
public:
  template <typename FF>
    requires(std::is_same_v<std::decay_t<FF>, F>)
  explicit scope_exit(FF&& fn) noexcept(std::is_nothrow_constructible_v<F, FF>)
  : fun_(std::forward<FF>(fn))
  {
  }

  scope_exit(scope_exit&& other) noexcept(std::is_nothrow_move_constructible_v<F>)
  : fun_(std::move(other.fun_)),
    active_(other.active_)
  {
    other.active_ = false;
  }

  ~scope_exit()
  {
    if (active_)
    {
      fun_();
    }
  }

  scope_exit(const scope_exit&) = delete;
  scope_exit& operator=(const scope_exit&) = delete;
  scope_exit& operator=(scope_exit&&) noexcept = delete;

private:
  F fun_;
  bool active_{true};
};

template <typename F>
scope_exit(F) -> scope_exit<F>;
} // namespace util
