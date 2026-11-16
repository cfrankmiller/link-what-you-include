// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <util/byte_visitor.hpp>
#include <util/byte_visitor/cstdint.hpp>

#include <cstdint>
#include <unordered_set>

namespace util
{
template <class Key, class Hash, class KeyEqual, class Allocator>
void visit(Byte_sink_visitor auto& v, const std::unordered_set<Key, Hash, KeyEqual, Allocator>& value)
{
  v(static_cast<uint64_t>(value.size()));
  for (const auto& e : value)
  {
    v(e);
  }
}

//template <class Key, class Hash, class KeyEqual, class Allocator>
//void visit(Byte_source_visitor auto& v, std::unordered_set<Key, Hash, KeyEqual, Allocator>& value)
//{
//  uint64_t length = 0U;
//  v(length);
//  if (!v)
//  {
//    return;
//  }
//
//  if (!v.can_read(static_cast<size_t>(length)))
//  {
//    v.set_error();
//    return;
//  }
//
//  std::unordered_set<Key, Hash, KeyEqual, Allocator> out;
//  if (0 < length)
//  {
//    const auto size = static_cast<size_t>(length);
//    for (size_t i = 0; i < size; ++i)
//    {
//      Key e;
//      v(e);
//      out.insert(std::move(e));
//    }
//  }
//
//  value.swap(out);
//}
} // namespace util
