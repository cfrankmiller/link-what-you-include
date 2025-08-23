// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <lwyi/command_options.hpp>

#include <tl/expected.hpp>

#include <string>

namespace lwyi
{
auto parse_arguments(int argc, const char* const* argv)
  -> tl::expected<Command_options, std::string>;
} // namespace lwyi
