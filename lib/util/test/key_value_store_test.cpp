// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#include <util/key_value_store.hpp>

#include <util/scope.hpp>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <print>
#include <source_location>
#include <string>

namespace
{
std::filesystem::path unique_tmp_dir(
  const std::source_location location = std::source_location::current())
{
  return std::filesystem::temp_directory_path() / std::format("key_value_store_test_{}", location.line());
}
} // namespace

TEST_CASE("util: key_value_store read/write test", "[util]")
{
  const auto cache_dir = unique_tmp_dir();
  util::scope_exit on_exit(
    [&cache_dir]()
    {
      std::error_code ec;
      std::filesystem::remove_all(cache_dir, ec);
    });

  const util::Hash_value key{};
  auto content = std::as_bytes(std::span("This is some content."));;

  auto store = util::default_key_value_store(cache_dir);
  auto write_result = store->write(key, content);
  REQUIRE(write_result);

  const auto read_result = store->read(key);
  if (!read_result.has_value())
  {
    std::print("error: {}\n", read_result.error());
  }
  REQUIRE(read_result);

  CHECK(std::ranges::equal(*read_result, content));
}

TEST_CASE("util: key_value_store read empty is not an error", "[util]")
{
  const auto cache_dir = unique_tmp_dir();

  auto store = util::default_key_value_store(cache_dir);

  const util::Hash_value key{};
  const auto read_result = store->read(key);
  REQUIRE(read_result);
  CHECK(read_result->empty());
}
