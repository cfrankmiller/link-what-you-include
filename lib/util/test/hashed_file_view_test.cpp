// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#include <util/hashed_file_view.hpp>

#include <util/scope.hpp>

#include <catch2/catch_test_macros.hpp>

#include <fstream>
#include <source_location>

namespace
{
std::filesystem::path unique_tmp_dir(
  const std::source_location location = std::source_location::current())
{
  return std::filesystem::temp_directory_path() /
         std::format("file_hasher_test_{}", location.line());
}

void create_file(const std::filesystem::path& file, std::string_view content)
{
  std::ofstream output(file, std::ios::binary);
  output.write(content.data(), static_cast<std::streamsize>(content.size()));
}
} // namespace

TEST_CASE("util: Hashed_file_view", "[util]")
{
  const auto tmp_dir = unique_tmp_dir();
  util::scope_exit on_exit(
    [&tmp_dir]()
    {
      std::error_code ec;
      std::filesystem::remove_all(tmp_dir, ec);
    });
  std::error_code ec;
  std::filesystem::create_directories(tmp_dir, ec);
  REQUIRE(!ec);

  constexpr std::string_view content1 = "This is some file content\n";
  constexpr std::string_view content2 = "This is some other file content\n";

  const auto expected_hash1 = util::hash(content1);
  const auto expected_hash2 = util::hash(content2);

  const auto file1 = tmp_dir / "file1";
  const auto file2 = tmp_dir / "file2";
  const auto file3 = tmp_dir / "file3";

  create_file(file1, content1);
  create_file(file2, content2);

  util::Hashed_file_view hashed_file_view;
  auto ohash1 = hashed_file_view.hash_of(file1);
  REQUIRE(ohash1.has_value());
  CHECK(expected_hash1 == *ohash1);

  auto ohash2 = hashed_file_view.hash_of(file2);
  REQUIRE(ohash2.has_value());
  CHECK(expected_hash2 == *ohash2);

  auto ohash3 = hashed_file_view.hash_of(file3);
  CHECK(!ohash3.has_value());

  create_file(file1, content2);
  ohash1 = hashed_file_view.hash_of(file1);
  REQUIRE(ohash1.has_value());
  CHECK(expected_hash2 == *ohash1);
}
