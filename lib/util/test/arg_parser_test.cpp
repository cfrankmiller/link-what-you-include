// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#include <util/arg_parser.hpp>

#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <format>
#include <numeric>
#include <optional>
#include <print>
#include <string>
#include <string_view>
#include <vector>

namespace
{
using namespace std::string_view_literals;

auto to_string(std::vector<const char*> args)
{
  return std::format("command line: {}\n",
                     std::accumulate(args.begin(),
                                     args.end(),
                                     std::string{},
                                     [](const auto& lhs, const auto& rhs)
                                     {
                                       return std::format("{} {}", lhs, rhs);
                                     }));
}
} // namespace

TEST_CASE("util: arg_parser given no args", "[util]")
{
  struct Options
  {
  };
  auto parser = util::arg_parser<Options>();

  SECTION("no args")
  {
    const char* args = nullptr;
    auto result = parser.parse(args, args);
    REQUIRE(result.has_value());
  }
  SECTION("args")
  {
    std::vector<const char*> args{"-a"};
    auto result = parser.parse(args.begin(), args.end());
    REQUIRE(!result.has_value());
  }
}

TEST_CASE("util: arg_parser", "[util]")
{
  struct Options
  {
    bool a{false};
    bool b{false};
    std::string_view c{};
  };

  constexpr auto parser = util::arg_parser<Options>()
                            .arg("-a", "--aaa", &Options::a)
                            .arg("-b", "--bbb", &Options::b)
                            .arg("-c", "--ccc", &Options::c);

  SECTION("short option 1")
  {
    std::vector<const char*> args{"-a"};
    INFO(to_string(args));
    auto result = parser.parse(args.begin(), args.end());

    REQUIRE(result.has_value());
    CHECK(result.value().a == true);
    CHECK(result.value().b == false);
    CHECK(result.value().c == ""sv);
  }
  SECTION("short option 2")
  {
    std::vector<const char*> args{"-b"};
    INFO(to_string(args));
    auto result = parser.parse(args.begin(), args.end());

    REQUIRE(result.has_value());
    CHECK(result.value().a == false);
    CHECK(result.value().b == true);
    CHECK(result.value().c == ""sv);
  }
  SECTION("long option 1")
  {
    std::vector<const char*> args{"--aaa"};
    INFO(to_string(args));
    auto result = parser.parse(args.begin(), args.end());

    REQUIRE(result.has_value());
    CHECK(result.value().a == true);
    CHECK(result.value().b == false);
    CHECK(result.value().c == ""sv);
  }
  SECTION("long option 2")
  {
    std::vector<const char*> args{"--bbb"};
    INFO(to_string(args));
    auto result = parser.parse(args.begin(), args.end());

    REQUIRE(result.has_value());
    CHECK(result.value().a == false);
    CHECK(result.value().b == true);
    CHECK(result.value().c == ""sv);
  }
  SECTION("short parameter")
  {
    std::vector<const char*> args{"-c", "hello"};
    INFO(to_string(args));
    auto result = parser.parse(args.begin(), args.end());

    REQUIRE(result.has_value());
    CHECK(result.value().a == false);
    CHECK(result.value().b == false);
    CHECK(result.value().c == "hello");
  }
  SECTION("long parameter")
  {
    std::vector<const char*> args{"--ccc", "hello"};
    INFO(to_string(args));
    auto result = parser.parse(args.begin(), args.end());

    REQUIRE(result.has_value());
    CHECK(result.value().a == false);
    CHECK(result.value().b == false);
    CHECK(result.value().c == "hello");
  }
  SECTION("short parameter no space")
  {
    std::vector<const char*> args{"-chello"};
    INFO(to_string(args));
    auto result = parser.parse(args.begin(), args.end());

    REQUIRE(result.has_value());
    CHECK(result.value().a == false);
    CHECK(result.value().b == false);
    CHECK(result.value().c == "hello");
  }
  SECTION("long parameter no space")
  {
    std::vector<const char*> args{"--ccchello"};
    INFO(to_string(args));
    auto result = parser.parse(args.begin(), args.end());

    REQUIRE(!result.has_value());
  }
  SECTION("short parameter =")
  {
    std::vector<const char*> args{"-c=hello"};
    INFO(to_string(args));
    auto result = parser.parse(args.begin(), args.end());

    REQUIRE(result.has_value());
    CHECK(result.value().a == false);
    CHECK(result.value().b == false);
    CHECK(result.value().c == "=hello"); // intentional behavior
  }
  SECTION("long parameter =")
  {
    std::vector<const char*> args{"--ccc=hello"};
    INFO(to_string(args));
    auto result = parser.parse(args.begin(), args.end());

    REQUIRE(result.has_value());
    CHECK(result.value().a == false);
    CHECK(result.value().b == false);
    CHECK(result.value().c == "hello");
  }
  SECTION("missing short parameter")
  {
    std::vector<const char*> args{"-c", "-a"};
    INFO(to_string(args));
    auto result = parser.parse(args.begin(), args.end());

    REQUIRE(!result.has_value());
  }
  SECTION("missing long parameter")
  {
    std::vector<const char*> args{"--ccc", "-a"};
    INFO(to_string(args));
    auto result = parser.parse(args.begin(), args.end());

    REQUIRE(!result.has_value());
  }
  SECTION("unknown option")
  {
    std::vector<const char*> args{"-q"};
    INFO(to_string(args));
    auto result = parser.parse(args.begin(), args.end());

    REQUIRE(!result.has_value());
    CHECK(!result.error().empty());
  }
}

TEST_CASE("util: arg_parser given an array", "[util]")
{
  struct Options
  {
    bool a{false};
    bool b{false};
    std::string_view c;
    std::vector<std::string_view> d;
  };

  constexpr auto parser = util::arg_parser<Options>()
                            .arg("-a", "--aaa", &Options::a)
                            .arg("-b", "--bbb", &Options::b)
                            .arg("-c", "--ccc", &Options::c)
                            .arg("-d", &Options::d);

  SECTION("array at end of args")
  {
    std::vector<const char*> args{"-a", "-d", "one", "two", "three"};
    INFO(to_string(args));
    auto result = parser.parse(args.begin(), args.end());

    REQUIRE(result.has_value());
    CHECK(result.value().a == true);
    CHECK(result.value().b == false);
    CHECK(result.value().c == ""sv);

    std::vector<std::string_view> expected{"one", "two", "three"};
    CHECK(result.value().d == expected);
  }
  SECTION("array with other args after")
  {
    std::vector<const char*> args{"-d", "one", "two", "three", "-a"};
    INFO(to_string(args));
    auto result = parser.parse(args.begin(), args.end());

    REQUIRE(result.has_value());
    CHECK(result.value().a == true);
    CHECK(result.value().b == false);
    CHECK(result.value().c == ""sv);

    std::vector<std::string_view> expected{"one", "two", "three"};
    CHECK(result.value().d == expected);
  }
}

TEST_CASE("util: arg_parser given an int", "[util]")
{
  struct Options
  {
    bool a{false};
    bool b{false};
    std::string_view c;
    uint32_t d{};
  };

  constexpr auto parser = util::arg_parser<Options>()
                            .arg("-a", "--aaa", &Options::a)
                            .arg("-b", "--bbb", &Options::b)
                            .arg("-c", "--ccc", &Options::c)
                            .arg("-d", &Options::d);

  SECTION("int arg at end")
  {
    std::vector<const char*> args{"-a", "-d", "42"};
    INFO(to_string(args));
    auto result = parser.parse(args.begin(), args.end());

    if (!result.has_value())
    {
      std::print("!!! {}\n", result.error());
    }

    REQUIRE(result.has_value());
    CHECK(result.value().a == true);
    CHECK(result.value().b == false);
    CHECK(result.value().c == ""sv);

    CHECK(result.value().d == 42);
  }
  SECTION("int arg at beginning")
  {
    std::vector<const char*> args{"-d", "17", "-a"};
    INFO(to_string(args));
    auto result = parser.parse(args.begin(), args.end());

    REQUIRE(result.has_value());
    CHECK(result.value().a == true);
    CHECK(result.value().b == false);
    CHECK(result.value().c == ""sv);

    CHECK(result.value().d == 17);
  }
  SECTION("int arg with no space")
  {
    std::vector<const char*> args{"-d17", "-a"};
    INFO(to_string(args));
    auto result = parser.parse(args.begin(), args.end());

    REQUIRE(result.has_value());
    CHECK(result.value().a == true);
    CHECK(result.value().b == false);
    CHECK(result.value().c == ""sv);

    CHECK(result.value().d == 17);
  }
}

TEST_CASE("util: arg_parser given a terminal arg", "[util]")
{
  struct Options
  {
    bool a{false};
    bool b{false};
    std::string_view c;
    std::vector<std::string_view> d;
  };

  constexpr auto parser = util::arg_parser<Options>()
                            .arg("-a", "--aaa", &Options::a)
                            .arg("-b", "--bbb", &Options::b)
                            .arg("-c", "--ccc", &Options::c)
                            .terminal_arg("-d", &Options::d);

  SECTION("terminal arg after some args")
  {
    std::vector<const char*> args{"-a", "-d", "one", "two", "-b", "three"};
    INFO(to_string(args));
    auto result = parser.parse(args.begin(), args.end());

    REQUIRE(result.has_value());
    CHECK(result.value().a == true);
    CHECK(result.value().b == false);
    CHECK(result.value().c == ""sv);

    std::vector<std::string_view> expected{"one", "two", "-b", "three"};
    CHECK(result.value().d == expected);
  }
  SECTION("terminal arg first")
  {
    std::vector<const char*> args{"-d", "one", "two", "-b", "three"};
    INFO(to_string(args));
    auto result = parser.parse(args.begin(), args.end());

    REQUIRE(result.has_value());
    CHECK(result.value().a == false);
    CHECK(result.value().b == false);
    CHECK(result.value().c == ""sv);

    std::vector<std::string_view> expected{"one", "two", "-b", "three"};
    CHECK(result.value().d == expected);
  }
}

TEST_CASE("util: arg_parser given an optional<string>", "[util]")
{
  struct Options
  {
    bool a{false};
    std::optional<std::string> b;
    uint32_t c{0U};
  };

  constexpr auto parser = util::arg_parser<Options>() //
                            .arg("-a", "--aaa", &Options::a)
                            .arg("-b", "--bbb", &Options::b)
                            .arg("-c", "--ccc", &Options::c);

  SECTION("optional arg not passed")
  {
    std::vector<const char*> args{"-a", "-c42"};
    INFO(to_string(args));
    auto result = parser.parse(args.begin(), args.end());

    if (!result.has_value())
    {
      std::print("!!! {}\n", result.error());
    }

    REQUIRE(result.has_value());
    CHECK(result.value().a == true);
    CHECK(result.value().b == std::nullopt);
    CHECK(result.value().c == 42);
  }
  SECTION("optional arg without value")
  {
    std::vector<const char*> args{"-a", "-b", "-c42"};
    INFO(to_string(args));
    auto result = parser.parse(args.begin(), args.end());

    if (!result.has_value())
    {
      std::print("!!! {}\n", result.error());
    }

    REQUIRE(result.has_value());
    CHECK(result.value().a == true);
    CHECK(result.value().b == "");
    CHECK(result.value().c == 42);
  }
  SECTION("optional arg without value at end")
  {
    std::vector<const char*> args{"-a", "-c42", "-b"};
    INFO(to_string(args));
    auto result = parser.parse(args.begin(), args.end());

    if (!result.has_value())
    {
      std::print("!!! {}\n", result.error());
    }

    REQUIRE(result.has_value());
    CHECK(result.value().a == true);
    CHECK(result.value().b == "");
    CHECK(result.value().c == 42);
  }
  SECTION("optional arg with a value")
  {
    std::vector<const char*> args{"-a", "-b", "yep", "-c42"};
    INFO(to_string(args));
    auto result = parser.parse(args.begin(), args.end());

    if (!result.has_value())
    {
      std::print("!!! {}\n", result.error());
    }

    REQUIRE(result.has_value());
    CHECK(result.value().a == true);
    CHECK(result.value().b == "yep");
    CHECK(result.value().c == 42);
  }
  SECTION("optional arg with a value at end")
  {
    std::vector<const char*> args{"-a", "-c42", "-b", "yep"};
    INFO(to_string(args));
    auto result = parser.parse(args.begin(), args.end());

    if (!result.has_value())
    {
      std::print("!!! {}\n", result.error());
    }

    REQUIRE(result.has_value());
    CHECK(result.value().a == true);
    CHECK(result.value().b == "yep");
    CHECK(result.value().c == 42);
  }
  SECTION("optional arg with a value using =")
  {
    std::vector<const char*> args{"--bbb=yep", "-a"};
    INFO(to_string(args));
    auto result = parser.parse(args.begin(), args.end());

    if (!result.has_value())
    {
      std::print("!!! {}\n", result.error());
    }

    REQUIRE(result.has_value());
    CHECK(result.value().a == true);
    CHECK(result.value().b == "yep");
  }
  SECTION("optional arg without value using =")
  {
    std::vector<const char*> args{"--bbb=", "-a"};
    INFO(to_string(args));
    auto result = parser.parse(args.begin(), args.end());

    if (!result.has_value())
    {
      std::print("!!! {}\n", result.error());
    }

    REQUIRE(result.has_value());
    CHECK(result.value().a == true);
    CHECK(result.value().b == "");
  }
}
