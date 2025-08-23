// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#include <util/parallel_transformer.hpp>

#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <numeric>
#include <sstream>
#include <thread>
#include <vector>

TEST_CASE("util: basic parallel_transformer test", "[util]")
{
  constexpr size_t count = 100;
  std::vector<int> x(count);
  std::iota(x.begin(), x.end(), 0);
  std::vector<int> y(x.size(), 0);

  const size_t thread_count = 7;
  util::Parallel_transformer transformer(thread_count);
  transformer.transform(x.begin(),
                        x.end(),
                        y.begin(),
                        [](int x)
                        {
                          std::this_thread::sleep_for(std::chrono::milliseconds(1));
                          return x / 2;
                        });

  std::vector<int> z(x.size(), 0);
  std::transform(x.begin(),
                 x.end(),
                 z.begin(),
                 [](int x)
                 {
                   return x / 2;
                 });

  bool ok = true;
  for (size_t i = 0; i < y.size(); ++i)
  {
    INFO(i);
    if (y[i] != z[i])
    {
      ok = false;
    }
  }

  CHECK(ok);
}

TEST_CASE("util: parallel_transformer uses threads", "[util]")
{
  constexpr size_t count = 100;
  std::vector<int> v(count);
  std::iota(v.begin(), v.end(), 0);

  std::vector<std::thread::id> out(v.size(), std::thread::id{});

  const size_t thread_count = 3;
  util::Parallel_transformer transformer(thread_count);
  transformer.transform(v.begin(),
                        v.end(),
                        out.begin(),
                        [](int)
                        {
                          std::this_thread::sleep_for(std::chrono::milliseconds(1));
                          return std::this_thread::get_id();
                        });

  std::sort(out.begin(), out.end());
  out.erase(std::unique(out.begin(), out.end()), out.end());

  std::ostringstream os;
  for (auto i : out)
  {
    os << i << ", ";
  }
  INFO(os.str());
  REQUIRE(out.size() == thread_count);
}
