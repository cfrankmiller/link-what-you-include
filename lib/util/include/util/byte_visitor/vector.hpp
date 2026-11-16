// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <util/byte_visitor.hpp>
#include <util/byte_visitor/cstdint.hpp>

#include <cstdint>
#include <vector>

namespace util
{
template <class T>
void visit(Byte_sink_visitor auto& v, const std::vector<T>& value)
{
  v(static_cast<uint64_t>(value.size()));
  for (const auto& e : value)
  {
    v(e);
  }
}

template <class T>
void visit(Byte_source_visitor auto& v, std::vector<T>& value)
{
  uint64_t length = 0U;
  v(length);

  if (!v || !v.can_read(length))
  {
    return;
  }

  const auto size = static_cast<size_t>(length);
  std::vector<T> out;
  out.reserve(size);
  for (size_t i = 0; i < size; ++i)
  {
    T e;
    v(e);
    out.push_back(std::move(e));
  }
  value.swap(out);
}
} // namespace util
