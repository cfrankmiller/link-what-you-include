// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <tl/expected.hpp>

#include <cstdint>
#include <string_view>
#include <vector>

namespace lwyi
{
struct Command_options
{
  std::string_view binary_dir;
  std::vector<std::string_view> targets;
  std::vector<std::string_view> tool_command;
  uint32_t num_threads;
};
} // namespace lwyi
