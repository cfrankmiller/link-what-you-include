// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#include <lwyi/dependency_visibility.hpp>

#include <cstdint>

namespace lwyi
{
Dependency_visibility_class compute_dependency_visibility_class(const Dependency_visibility& visibility)
{
  const auto include_scope = static_cast<uint8_t>(visibility.include_scope);
  const auto link_scope = static_cast<uint8_t>(visibility.link_scope);
  const auto over_include = static_cast<uint8_t>(!!(include_scope & ~link_scope));
  const auto over_link = static_cast<uint8_t>(!!(link_scope & ~include_scope));
  const auto result = (over_include << 1U) | over_link;
  return static_cast<Dependency_visibility_class>(result);
}
} // namespace lwyi
