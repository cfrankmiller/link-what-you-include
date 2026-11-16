// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#include <src/byte_visitors.hpp>

#include <scanner/include.hpp>
#include <util/byte_reader.hpp>
#include <util/byte_visitor/cstdint.hpp>
#include <util/byte_visitor/string.hpp>
#include <util/byte_writer.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <filesystem>
#include <string>

TEST_CASE("scanner: Source_line serialization", "[scanner]")
{
  util::Byte_writer output;

  const std::filesystem::path s1 = "/path/to/a/source_1.cpp";
  const std::filesystem::path s2 = "/path/to/a/source_2.cpp";
  const std::filesystem::path s3 = "/path/to/a/source_3.cpp";
  const uint32_t l1 = 17U;
  const uint32_t l2 = 42U;
  const uint32_t l3 = 3U;
  const scanner::Source_line sl1{s1, l1};
  const scanner::Source_line sl2{s2, l2};
  const scanner::Source_line sl3{s3, l3};

  const int v1 = 1;
  const std::string v2 = "this is a string";
  const int v3 = 3;

  output(v1);
  output(sl1);
  output(sl2);
  output(v2);
  output(sl3);
  output(v3);
  REQUIRE(output);

  util::Byte_reader input(output.view_buffer());

  scanner::Source_line rsl1;
  scanner::Source_line rsl2;
  scanner::Source_line rsl3;
  int r1 = 0;
  std::string r2;
  int r3 = 0;

  input(r1);
  input(rsl1);
  input(rsl2);
  input(r2);
  input(rsl3);
  input(r3);
  REQUIRE(input);

  CHECK(sl1.source == rsl1.source);
  CHECK(sl1.line == rsl1.line);
  CHECK(sl2.source == rsl2.source);
  CHECK(sl2.line == rsl2.line);
  CHECK(sl3.source == rsl3.source);
  CHECK(sl3.line == rsl3.line);
  CHECK(r1 == v1);
  CHECK(r2 == v2);
  CHECK(r3 == v3);
}

TEST_CASE("scanner: include serialization", "[scanner]")
{
  util::Byte_writer output;

  scanner::Include inc{"/path/to/a/file.cpp", {}};

  output(inc);

  util::Byte_reader input(output.view_buffer());

  scanner::Include rinc;
  input(rinc);
  REQUIRE(input);

  CHECK(rinc.path == inc.path);
}
