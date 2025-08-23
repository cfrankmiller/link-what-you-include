// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <string>

namespace target_model
{
struct Target
{
  std::string name;

  friend auto operator==(const Target& lhs, const Target& rhs) -> bool
  {
    return lhs.name == rhs.name;
  }

  friend auto operator<(const Target& lhs, const Target& rhs) -> bool
  {
    return lhs.name < rhs.name;
  }
};
} // namespace target_model

template <>
struct std::hash<target_model::Target>
{
  auto operator()(const target_model::Target& target) const noexcept -> std::size_t
  {
    return std::hash<std::string>{}(target.name);
  }
};
