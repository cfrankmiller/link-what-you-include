// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#include <src/cluster_diff.hpp>

#include <target_model/target.hpp>

#include <catch2/catch_test_macros.hpp>

#include <set>
#include <utility>
#include <vector>

TEST_CASE("tidy: cluster_diff", "[tidy]")
{
  GIVEN("two lists of clusters that match")
  {
    std::vector<std::set<target_model::Target>> lhs{{{"a"}, {"b"}, {"c"}},
                                                    {{"d"}, {"e"}},
                                                    {{"f"}, {"g"}}};
    std::vector<std::set<target_model::Target>> rhs{{{"a"}, {"b"}, {"c"}},
                                                    {{"d"}, {"e"}},
                                                    {{"f"}, {"g"}}};

    WHEN("cluster_diff is called")
    {
      auto result = tidy::cluster_diff(lhs, rhs);

      THEN("there should be no differences")
      {
        REQUIRE(result.size() == 3);

        CHECK(result[0].left_only.empty());
        CHECK(result[0].right_only.empty());
        CHECK(result[1].left_only.empty());
        CHECK(result[1].right_only.empty());
        CHECK(result[2].left_only.empty());
        CHECK(result[2].right_only.empty());
      }
    }

    WHEN("the order of the clusters is different")
    {
      std::swap(rhs[0], rhs[1]);
      auto result = tidy::cluster_diff(lhs, rhs);

      THEN("there should be no differences")
      {
        REQUIRE(result.size() == 3);

        CHECK(result[0].left_only.empty());
        CHECK(result[0].right_only.empty());
        CHECK(result[1].left_only.empty());
        CHECK(result[1].right_only.empty());
        CHECK(result[2].left_only.empty());
        CHECK(result[2].right_only.empty());
      }
    }

    WHEN("a target is added to one of the clusters")
    {
      rhs[0].insert({"x"});

      auto result = tidy::cluster_diff(lhs, rhs);

      THEN("there should be differences")
      {
        REQUIRE(result.size() == 3);

        CHECK(result[0].left_only.empty());
        CHECK(result[0].right_only.size() == 1);
        CHECK(result[1].left_only.empty());
        CHECK(result[1].right_only.empty());
        CHECK(result[2].left_only.empty());
        CHECK(result[2].right_only.empty());

        CHECK(result[0].right_only == std::vector<target_model::Target>{{"x"}});
      }
    }

    WHEN("a target is removed from one of the clusters")
    {
      rhs[0].erase({"b"});

      auto result = tidy::cluster_diff(lhs, rhs);

      THEN("there should be differences")
      {
        REQUIRE(result.size() == 3);

        CHECK(result[0].left_only.size() == 1);
        CHECK(result[0].right_only.empty());
        CHECK(result[1].left_only.empty());
        CHECK(result[1].right_only.empty());
        CHECK(result[2].left_only.empty());
        CHECK(result[2].right_only.empty());

        CHECK(result[0].left_only == std::vector<target_model::Target>{{"b"}});
      }
    }

    WHEN("a new cluster is added")
    {
      rhs.push_back({{"x"}, {"y"}});

      auto result = tidy::cluster_diff(lhs, rhs);

      THEN("there should be differences")
      {
        REQUIRE(result.size() == 4);

        CHECK(result[0].left_only.empty());
        CHECK(result[0].right_only.empty());
        CHECK(result[1].left_only.empty());
        CHECK(result[1].right_only.empty());
        CHECK(result[2].left_only.empty());
        CHECK(result[2].right_only.empty());
        CHECK(result[3].left_only.empty());
        CHECK(result[3].right_only.size() == 2);

        CHECK(result[3].right_only == std::vector<target_model::Target>{{"x"}, {"y"}});
      }
    }

    WHEN("a cluster is removed")
    {
      rhs.pop_back();
      auto result = tidy::cluster_diff(lhs, rhs);

      THEN("there should be differences")
      {
        REQUIRE(result.size() == 3);

        CHECK(result[0].left_only.empty());
        CHECK(result[0].right_only.empty());
        CHECK(result[1].left_only.empty());
        CHECK(result[1].right_only.empty());
        CHECK(result[2].left_only.size() == 2);
        CHECK(result[2].right_only.empty());

        CHECK(result[2].left_only == std::vector<target_model::Target>{{"f"}, {"g"}});
      }
    }
  }
}
