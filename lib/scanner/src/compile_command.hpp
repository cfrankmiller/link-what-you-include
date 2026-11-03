// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace scanner
{
struct Compile_command
{
  std::filesystem::path cwd;
  std::filesystem::path source;
  std::vector<std::string> command;
};
} // namespace scanner
