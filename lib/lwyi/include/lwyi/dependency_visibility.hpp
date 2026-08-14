// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <lwyi/dependency_scope.hpp>

#include <cstdint>

namespace lwyi
{
struct Dependency_visibility
{
  Dependency_scope include_scope{Dependency_scope::none};
  Dependency_scope link_scope{Dependency_scope::none};
};

enum class Dependency_visibility_class : uint8_t
{
  good = 0b00,
  over_link = 0b01,
  over_include = 0b10,
  bad = 0b11
};

Dependency_visibility_class compute_dependency_visibility_class(
  const Dependency_visibility& visibility);
} // namespace lwyi
