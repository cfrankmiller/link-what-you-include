// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#include <util/scope.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("util: scope_exit", "[util]")
{
  int i = 0;
  {
    util::scope_exit on_exit(
      [&i]
      {
        ++i;
      });
  }
  CHECK(i == 1);
}
