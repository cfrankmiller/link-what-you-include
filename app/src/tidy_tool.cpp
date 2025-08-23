// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#include <target_model/target.hpp>
#include <tidy/load_config.hpp>
#include <tidy/tidy.hpp>
#include <util/arg_parser.hpp>

#include <fmt/base.h>
#include <tl/expected.hpp>

#include <cassert>
#include <cstddef>
#include <filesystem>
#include <string_view>
#include <vector>

namespace target_model
{
class Target_model;
} // namespace target_model

namespace
{
constexpr std::string_view usage_string = R"(Usage:
  tidy [options]

Possible options:
  -h, --help                Print this help message.
  -c, --config FILE         Path to config file.)";

struct Options
{
  bool help{false};
  std::string_view config_filename;
};

constexpr auto parser = util::arg_parser<Options>()
                          .arg("-h", "--help", &Options::help)
                          .arg("-c", "--config", &Options::config_filename);
} // namespace

auto tidy_tool(const target_model::Target_model& target_model,
               const std::vector<target_model::Target>& /*selected_targets*/,
               const std::vector<std::string_view>& args) -> int
{
  assert(!args.empty() && args.front() == "tidy");

  auto result = parser.parse(args.begin() + 1, args.end());

  if (!result.has_value())
  {
    fmt::print("{}\n{}\n", result.error(), usage_string);
    return 1;
  }

  const auto& options = result.value();

  if (options.help)
  {
    fmt::print("{}\n", usage_string);
    return 1;
  }
  if (options.config_filename.empty())
  {
    fmt::print("A config file is required.\n{}\n", usage_string);
    return 1;
  }

  auto config = tidy::load_config(options.config_filename);
  if (!config.has_value())
  {
    fmt::print("Failed to load config file.\n{}\n", config.error());
    return 1;
  }

  auto diagnostics = tidy::tidy(config.value(), target_model);

  auto print = [](std::vector<target_model::Target> targets)
  {
    for (size_t i = 0; i < targets.size(); ++i)
    {
      if (i != 0)
      {
        if (2 < targets.size())
        {
          fmt::print(",");
        }
        fmt::print(" ");
      }
      if (i + 1 == targets.size() && 1 < targets.size())
      {
        fmt::print("and ");
      }
      fmt::print("{}", targets[i].name);
    }
  };

  bool error = false;
  for (const auto& diagnostic : diagnostics)
  {
    switch (diagnostic.type)
    {
      case tidy::Dag_diagnostic_type::added_to_cluster:
      {
        fmt::print("error: a known target cluster increased in size with the addition of ");
        print(diagnostic.targets);
        fmt::print("\n\n");
        error = true;
      }
      break;
      case tidy::Dag_diagnostic_type::removed_from_cluster:
      {
        fmt::print("Warning: a known target cluster decreased in size with the removal of ");
        print(diagnostic.targets);
        fmt::print("\n\n");
      }
      break;
      case tidy::Dag_diagnostic_type::new_cluster:
      {
        fmt::print("Warning: a new target cluster was introduced with ");
        print(diagnostic.targets);
        fmt::print("\n\n");
        error = true;
      }
      break;
      case tidy::Dag_diagnostic_type::forbidden_dependency:
      {
        assert(diagnostic.targets.size() == 2);
        fmt::print("error: {} is forbidden to depend on {}\n",
                   diagnostic.targets[0].name,
                   diagnostic.targets[1].name);
        error = true;
      }
      break;
    }
  }

  return error ? 1 : 0;
}
