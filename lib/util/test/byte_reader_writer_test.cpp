// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#include <util/byte_reader.hpp>
#include <util/byte_writer.hpp>
#include <util/byte_visitor/cstdint.hpp>
#include <util/byte_visitor/filesystem.hpp>
#include <util/byte_visitor/set.hpp>
#include <util/byte_visitor/string.hpp>
#include <util/byte_visitor/vector.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <filesystem>
#include <set>
#include <string>
#include <vector>

TEST_CASE("util: primitive serialization", "[util]")
{
  util::Byte_writer output;

  const int32_t v1 = 17;
  const uint64_t v2 = 42;
  const std::string v3 = "this is a string";
  const std::string v4 = "this is another string";
  const int32_t v5 = -17;
  const std::filesystem::path v6 = "/this/is/a/path.hpp";
  const std::filesystem::path v7 = "/this/is/another/path.hpp";
  const int16_t v8 = -42;
  output(v1);
  output(v2);
  output(v3);
  output(v4);
  output(v5);
  output(v6);
  output(v7);
  output(v8);
  REQUIRE(output);

  util::Byte_reader input(output.view_buffer());
  int32_t r1 = 0;
  uint64_t r2 = 0;
  std::string r3;
  std::string r4;
  int32_t r5 = 0;
  std::filesystem::path r6;
  std::filesystem::path r7;
  int16_t r8 = 0;

  input(r1);
  input(r2);
  input(r3);
  input(r4);
  input(r5);
  input(r6);
  input(r7);
  input(r8);
  REQUIRE(input);

  CHECK(r1 == v1);
  CHECK(r2 == v2);
  CHECK(r3 == v3);
  CHECK(r4 == v4);
  CHECK(r5 == v5);
  CHECK(r6 == v6);
  CHECK(r7 == v7);
  CHECK(r8 == v8);
}

TEST_CASE("util: short buffer deserialization error", "[util]")
{
  std::string buffer("bla bla");
  util::Byte_reader input(std::as_bytes(std::span(buffer.data(), buffer.size())));
  int r1 = 0;
  int r2 = 0;

  input(r1);
  input(r2);
  REQUIRE(!input);
}

TEST_CASE("util: bad string deserialization error", "[util]")
{
  // Make sure that if we are deserializing garbage that we don't try to allocate a string that is obviously too big
  std::vector<std::byte> buffer(42, std::byte{0xff});
  util::Byte_reader input(buffer);
  std::string r1;
  input(r1);
  REQUIRE(!input);
}

TEST_CASE("util: path serialization", "[util]")
{
  std::filesystem::path p("/path/to/a/file.thing");

  util::Byte_writer output;
  output(p);
  REQUIRE(output);

  util::Byte_reader input(output.view_buffer());

  std::filesystem::path rp;
  input(rp);
  REQUIRE(input);

  CHECK(rp == p);
}

TEST_CASE("util: vector serialization", "[util]")
{
  std::vector<int> v{1, 17, 42, -17, -1};

  util::Byte_writer output;
  output(v);
  REQUIRE(output);

  util::Byte_reader input(output.view_buffer());

  std::vector<int> rv;
  input(rv);
  REQUIRE(input);

  CHECK(rv == v);
}

TEST_CASE("util: set serialization", "[util]")
{
  std::set<int> s{1, 17, 42, -17, -1};

  util::Byte_writer output;
  output(s);
  REQUIRE(output);

  util::Byte_reader input(output.view_buffer());

  std::set<int> rs;
  input(rs);
  REQUIRE(input);

  CHECK(rs == s);
}
