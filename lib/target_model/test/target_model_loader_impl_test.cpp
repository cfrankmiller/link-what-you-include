// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#include <src/target_model_loader_impl.hpp>

#include <src/file_loader.hpp>
#include <target_model/target.hpp>
#include <target_model/target_data.hpp>
#include <target_model/target_model.hpp>

#include <catch2/catch_test_macros.hpp>
#include <simdjson.h>
#include <tl/expected.hpp>

#include <cstring>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <regex>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace
{
class Test_file_loader : public target_model::File_loader
{
public:
  explicit Test_file_loader(const char* json)
  : size_(std::strlen(json))
  {
    bytes_.resize(size_ + simdjson::SIMDJSON_PADDING);
    std::memcpy(bytes_.data(), json, size_);
  }

  [[nodiscard]] auto load(const std::filesystem::path& path)
    -> tl::expected<void, std::string> override
  {
    static_cast<void>(path);
    return {};
  }

  [[nodiscard]] auto data() const -> const char* override
  {
    return bytes_.data();
  }

  [[nodiscard]] auto size() const -> size_t override
  {
    return size_;
  }

  [[nodiscard]] auto size_with_padding() const -> size_t override
  {
    return bytes_.size();
  }

private:
  size_t size_{0U};
  std::vector<char> bytes_;
};
} // namespace

TEST_CASE("target_model: target_model_loader_impl can load a valid json file", "[target_model]")
{
  const char* json = R"===({
    "liba": {
      "interface_include_directories": [],
      "interface_headers": [
        "/some/path/liba/one.h",
        "/some/path/liba/two.h"
      ],
      "interface_dependencies": [],
      "dependencies": [],
      "sources": [
        "/some/other/path/liba/one.cpp",
        "/some/other/path/liba/two.cpp"
      ],
      "verify_interface_header_sets_sources": [
        "/some/path/liba/one.h.cpp",
        "/some/path/liba/two.h.cpp"
      ]
    },
    "libb": {
      "interface_include_directories": [],
      "interface_headers": [
        "/some/path/libb/one.h",
        "/some/path/libb/two.h"
      ],
      "interface_dependencies": [],
      "dependencies": [
        "liba"
      ],
      "sources": []
    },
    "libc": {
      "interface_include_directories": [
        "/some/path/libc/",
        "/other/path/libc/"
      ],
      "interface_headers": [],
      "interface_dependencies": [
        "libb"
      ],
      "dependencies": [
        "libb"
      ]
    }
  })===";

  auto file_loader = std::make_unique<Test_file_loader>(json);
  target_model::Target_model_loader_impl target_model_loader(std::move(file_loader));
  auto result = target_model_loader.load_json("/some/file.json");
  REQUIRE(result.has_value());

  auto target_model = target_model_loader.make_target_model();

  auto data = target_model.get_target_data(target_model::Target{"liba"});
  REQUIRE(data.has_value());
  const auto& liba_info = data.value().get(); // NOLINT(bugprone-unchecked-optional-access)
  CHECK(liba_info.interface_include_directories.empty());
  CHECK(liba_info.interface_headers ==
        std::unordered_set<std::filesystem::path, target_model::Path_hash>{
          "/some/path/liba/one.h",
          "/some/path/liba/two.h"});
  CHECK(liba_info.interface_dependencies.empty());
  CHECK(liba_info.sources ==
        std::unordered_set<std::filesystem::path, target_model::Path_hash>{
          "/some/other/path/liba/one.cpp",
          "/some/other/path/liba/two.cpp"});
  CHECK(liba_info.verify_interface_header_sets_sources ==
        std::unordered_set<std::filesystem::path, target_model::Path_hash>{
          "/some/path/liba/one.h.cpp",
          "/some/path/liba/two.h.cpp"});
  CHECK(liba_info.dependencies.empty());

  data = target_model.get_target_data(target_model::Target{"libb"});
  REQUIRE(data.has_value());
  const auto& libb_info = data.value().get(); // NOLINT(bugprone-unchecked-optional-access)
  CHECK(libb_info.interface_include_directories.empty());
  CHECK(libb_info.interface_headers ==
        std::unordered_set<std::filesystem::path, target_model::Path_hash>{
          "/some/path/libb/one.h",
          "/some/path/libb/two.h"});
  CHECK(libb_info.interface_dependencies.empty());
  CHECK(libb_info.sources.empty());
  CHECK(libb_info.dependencies == std::unordered_set<target_model::Target>{{"liba"}});

  data = target_model.get_target_data(target_model::Target{"libc"});
  REQUIRE(data.has_value());
  const auto& libc_info = data.value().get(); // NOLINT(bugprone-unchecked-optional-access)
  CHECK(
    libc_info.interface_include_directories ==
    std::unordered_set<std::filesystem::path, target_model::Path_hash>{"/some/path/libc/",
                                                                       "/other/path/libc/"});
  CHECK(libc_info.interface_headers.empty());
  CHECK(libc_info.interface_dependencies ==
        std::unordered_set<target_model::Target>{{"libb"}});
  CHECK(libc_info.sources.empty());
  CHECK(libc_info.dependencies == std::unordered_set<target_model::Target>{{"libb"}});

  CHECK(target_model.map_header_to_target("/some/path/liba/one.h") ==
        target_model::Target{"liba"});
  CHECK(target_model.map_header_to_target("/some/path/liba/two.h") ==
        target_model::Target{"liba"});
  CHECK(target_model.map_header_to_target("/some/path/libb/one.h") ==
        target_model::Target{"libb"});
  CHECK(target_model.map_header_to_target("/some/path/libb/two.h") ==
        target_model::Target{"libb"});
  CHECK(target_model.map_header_to_target("/some/path/libc/one.h") ==
        target_model::Target{"libc"});
  CHECK(target_model.map_header_to_target("/other/path/libc/two.h") ==
        target_model::Target{"libc"});
}

TEST_CASE("target_model: target_model_loader_impl will fail to load an invalid json file",
          "[target_model]")
{
  const char* json = R"===({
    "liba": {
      "interface_include_directories": [],
      "dependencies": []
    },
    "libb": [
        "/some/path/libb/one.h",
        "/some/path/libb/two.h"
      ]
    }
  })===";

  auto file_loader = std::make_unique<Test_file_loader>(json);
  target_model::Target_model_loader_impl target_model_loader(std::move(file_loader));
  auto result = target_model_loader.load_json("/some/file.json");
  REQUIRE(!result.has_value());

  std::regex message_regex("error.*: line \\d+, column \\d+: ");
  CHECK(std::regex_search(result.error(), message_regex));
}
