// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#include <lwyi/dependency_visibility.hpp>

#include <lwyi/dependency_scope.hpp>

#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>

#include <format>
#include <utility>
#include <vector>

TEST_CASE("lwyi: compute_dependency_visibility_class", "[lwyi]")
{
  using namespace lwyi;

  // scope                     class
  //   00    none                00    good
  //   01    private             01    over_link
  //   10    interface           10    over_include
  //   11    public              11    bad
  //
  // include scope    link scope        class
  //   00 none          00 none           00  good
  //   00 none          01 private        01  over_link
  //   00 none          10 interface      01  over_link
  //   00 none          11 public         01  over_link
  //
  //   01 private       00 none           10  over_include
  //   01 private       01 private        00  good
  //   01 private       10 interface      11  bad
  //   01 private       11 public         01  over_link
  //
  //   10 interface     00 none           10  over_include
  //   10 interface     01 private        11  bad
  //   10 interface     10 interface      00  good
  //   10 interface     11 public         01  over_link
  //
  //   11 public        00 none           10  over_include
  //   11 public        01 private        10  over_include
  //   11 public        10 interface      10  over_include
  //   11 public        11 public         00  good

  // clang-format off
  std::vector<std::pair<lwyi::Dependency_visibility, lwyi::Dependency_visibility_class>> test_data = {
     { { Dependency_scope::none,            Dependency_scope::none            }, Dependency_visibility_class::good         },
     { { Dependency_scope::none,            Dependency_scope::private_scope   }, Dependency_visibility_class::over_link    },
     { { Dependency_scope::none,            Dependency_scope::interface_scope }, Dependency_visibility_class::over_link    },
     { { Dependency_scope::none,            Dependency_scope::public_scope    }, Dependency_visibility_class::over_link    },

     { { Dependency_scope::private_scope,   Dependency_scope::none            }, Dependency_visibility_class::over_include },
     { { Dependency_scope::private_scope,   Dependency_scope::private_scope   }, Dependency_visibility_class::good         },
     { { Dependency_scope::private_scope,   Dependency_scope::interface_scope }, Dependency_visibility_class::bad          },
     { { Dependency_scope::private_scope,   Dependency_scope::public_scope    }, Dependency_visibility_class::over_link    },

     { { Dependency_scope::interface_scope, Dependency_scope::none            }, Dependency_visibility_class::over_include },
     { { Dependency_scope::interface_scope, Dependency_scope::private_scope   }, Dependency_visibility_class::bad          },
     { { Dependency_scope::interface_scope, Dependency_scope::interface_scope }, Dependency_visibility_class::good         },
     { { Dependency_scope::interface_scope, Dependency_scope::public_scope    }, Dependency_visibility_class::over_link    },

     { { Dependency_scope::public_scope,    Dependency_scope::none            }, Dependency_visibility_class::over_include },
     { { Dependency_scope::public_scope,    Dependency_scope::private_scope   }, Dependency_visibility_class::over_include },
     { { Dependency_scope::public_scope,    Dependency_scope::interface_scope }, Dependency_visibility_class::over_include },
     { { Dependency_scope::public_scope,    Dependency_scope::public_scope    }, Dependency_visibility_class::good         }
  };
  // clang-format on

  for (const auto& [visibility, expected_class] : test_data)
  {
    const auto result = compute_dependency_visibility_class(visibility);
    INFO(std::format("expected: ({:02b}, {:02b}) = {:02b}\n",
                     std::to_underlying(visibility.include_scope),
                     std::to_underlying(visibility.link_scope),
                     std::to_underlying(expected_class)));
    INFO(std::format("result:   ({:02b}, {:02b}) = {:02b}\n",
                     std::to_underlying(visibility.include_scope),
                     std::to_underlying(visibility.link_scope),
                     std::to_underlying(result)));
    CHECK(result == expected_class);
  }
}
