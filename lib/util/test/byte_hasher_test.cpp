// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#include <util/byte_hasher.hpp>
#include <util/byte_visitor/cstdint.hpp>
#include <util/byte_visitor/string.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <string>

TEST_CASE("util: Byte_hasher", "[util]")
{
  const uint64_t v1 = 42;
  const std::string v2 = "this is a string";
  const int16_t v3 = -42;

  util::Byte_hasher hasher1;
  hasher1(v1);
  hasher1(v2);
  hasher1(v3);
  CHECK(hasher1);
  const auto hash1 = hasher1.digest();

  util::Byte_hasher hasher2;
  hasher2(v1);
  hasher2(v2);
  hasher2(v3);
  CHECK(hasher2);
  const auto hash2 = hasher2.digest();

  CHECK(hash1 == hash2);

  util::Byte_hasher hasher3;
  const std::string v2_modified = "this is a string!";
  hasher3(v1);
  hasher3(v2_modified);
  hasher3(v3);
  CHECK(hasher3);
  const auto hash3 = hasher3.digest();

  CHECK(hash1 != hash3);
}

TEST_CASE("util: copy Byte_hasher", "[util]")
{
  util::Byte_hasher hasher;

  hasher("this is a test");
  hasher("this is another test");

  util::Byte_hasher hasher2;
  hasher2 = hasher;
  CHECK(hasher.digest() == hasher2.digest());

  hasher("yet another test");
  CHECK(hasher.digest() != hasher2.digest());
}

TEST_CASE("util: Byte_hasher avoids the A,BC vs AB,C problem", "[util]")
{
  util::Byte_hasher hasher1;
  const std::string first1 = "A";
  const std::string second1 = "BC";
  hasher1(first1);
  hasher1(second1);
  CHECK(hasher1);
  const auto hash1 = hasher1.digest();

  util::Byte_hasher hasher2;
  const std::string first2 = "AB";
  const std::string second2 = "C";
  hasher2(first2);
  hasher2(second2);
  CHECK(hasher2);
  const auto hash2 = hasher2.digest();

  CHECK(hash1 != hash2);
}
