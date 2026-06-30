// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#include <lwyi/parse_arguments.hpp>
#include <src/run_lwyi.hpp>

#include <expected>
#include <print>
#include <string>

auto print_error(std::string error) -> std::expected<int, std::string>
{
  std::print("{}\n", error);
  return 1;
}

auto main(int argc, const char* argv[]) -> int
{
  return lwyi::parse_arguments(argc, argv).and_then(run_lwyi).or_else(print_error).value_or(1);
}
