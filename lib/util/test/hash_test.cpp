// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#include <util/hash.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>

TEST_CASE("util: hash", "[util]")
{
  std::string bytes1 = "this is a test";
  const auto result1 = util::hash(bytes1);

  std::string bytes2 = "this is another test";
  const auto result2 = util::hash(bytes2);

  CHECK(result1 != result2);
}
