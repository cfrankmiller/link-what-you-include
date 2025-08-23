// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#include <src/load_config_impl.hpp>

#include <target_model/target.hpp>
#include <tidy/config.hpp>

#include <catch2/catch_test_macros.hpp>
#include <simdjson.h>

#include <cstring>
#include <map>
#include <set>
#include <vector>

TEST_CASE("tidy: load a valid config", "[tidy]")
{
  GIVEN("a valid config file")
  {
    const char* json = R"===({
      "forbidden_dependencies": {
        "liba": ["libx", "liby"],
        "libb": ["libz"]
      },
      "allowed_clusters": [
        [
          "libq",
          "libr"
        ],
        [
          "libs",
          "libt",
          "libu"
        ]
      ]
      })===";
    simdjson::padded_string raw_config(json, std::strlen(json));

    WHEN("load_config_impl is called")
    {
      auto result = tidy::load_config_impl(raw_config);

      THEN("a valid config is returned")
      {
        REQUIRE(result.has_value());

        const auto& config = result.value();
        CHECK(config.forbidden_dependencies.at(target_model::Target{"liba"}) ==
              std::set<target_model::Target>{target_model::Target{"libx"},
                                             target_model::Target{"liby"}});
        CHECK(config.forbidden_dependencies.at(target_model::Target{"libb"}) ==
              std::set<target_model::Target>{target_model::Target{"libz"}});
        CHECK(config.allowed_clusters.at(0) ==
              std::set<target_model::Target>{target_model::Target{"libq"},
                                             target_model::Target{"libr"}});
        CHECK(config.allowed_clusters.at(1) ==
              std::set<target_model::Target>{target_model::Target{"libs"},
                                             target_model::Target{"libt"},
                                             target_model::Target{"libu"}});
      }
    }
  }
}
