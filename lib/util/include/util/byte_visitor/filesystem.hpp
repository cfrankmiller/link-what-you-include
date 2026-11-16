// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <util/byte_visitor.hpp>
#include <util/byte_visitor/string.hpp>

#include <filesystem>

namespace util
{
inline void visit(Byte_sink_visitor auto& v, const std::filesystem::path& path)
{
  v(path.string());
}

inline void visit(Byte_source_visitor auto& v, std::filesystem::path& path)
{
  std::string str;
  v(str);
  if (v)
  {
    path = str;
  }
}
} // namespace util
