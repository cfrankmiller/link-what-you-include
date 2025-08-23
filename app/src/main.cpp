// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#include <lwyi/parse_arguments.hpp>
#include <src/run_lwyi.hpp>

#include <fmt/base.h>
#include <tl/expected.hpp>

#include <string>

auto print_error(std::string error) -> tl::expected<int, std::string>
{
  fmt::print("{}\n", error);
  return 1;
}

auto main(int argc, const char* argv[]) -> int
{
  return lwyi::parse_arguments(argc, argv).and_then(run_lwyi).or_else(print_error).value_or(1);
}
