// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#include <src/run_lwyi.hpp>

#include <lwyi/command_options.hpp>
#include <src/run_lwyi_on_target.hpp>
#include <src/run_tool.hpp>
#include <target_model/target.hpp>
#include <target_model/target_model.hpp>
#include <target_model/target_model_loader.hpp>

#include <fmt/base.h>
#include <fmt/format.h>
#include <tl/expected.hpp>

#include <filesystem>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

namespace target_model
{
struct Target_data;
} // namespace target_model

auto run_lwyi(const lwyi::Command_options& options) -> tl::expected<int, std::string>
{
  auto working_dir = std::filesystem::current_path();
  auto binary_dir = working_dir;
  if (!options.binary_dir.empty())
  {
    binary_dir = options.binary_dir;
    if (!std::filesystem::is_directory(binary_dir))
    {
      return tl::unexpected(
        fmt::format("error: {} is not a directory\n", binary_dir.string()));
    }
  }

  const auto info_file = binary_dir / "link_what_you_include_info.json";
  if (!std::filesystem::is_regular_file(info_file))
  {
    return tl::unexpected(fmt::format("error: {} is not a file\n", info_file.string()));
  }

  fmt::print("# Loading build system info from {}\n", info_file.string());

  auto loader = target_model::Target_model_loader::create();
  const auto load_result = loader->load_json(info_file);
  if (!load_result.has_value())
  {
    return tl::unexpected(
      fmt::format("error: failed to load {}: {}\n", info_file.string(), load_result.error()));
  }
  const auto target_model = loader->make_target_model();

  std::vector<target_model::Target> selected_targets;
  selected_targets.reserve(options.targets.size());
  for (auto v : options.targets)
  {
    selected_targets.push_back({std::string(v)});
  }

  if (!options.tool_command.empty())
  {
    return run_tool(target_model, selected_targets, options.tool_command);
  }

  const auto num_threads = (0U < options.num_threads) ? options.num_threads
                                                      : std::thread::hardware_concurrency();
  fmt::print("Scanning with {} threads\n", num_threads);

  bool success = true;
  if (selected_targets.empty())
  {
    target_model.for_each_target(
      [&](const target_model::Target& target, const target_model::Target_data& target_data)
      {
        fmt::print("# Checking that {} links what it includes\n", target.name);
        std::cout.flush();

        success &=
          run_lwyi_on_target(target_model, binary_dir, target, target_data, num_threads);
      });
  }
  else
  {
    for (const auto& target : selected_targets)
    {
      fmt::print("# Checking that {} links what it includes\n", target.name);
      std::cout.flush();

      auto otarget_data = target_model.get_target_data(target);
      if (!otarget_data.has_value())
      {
        fmt::print("error: No target named {} found\n", target.name);
        success = false;
        break;
      }

      success &=
        run_lwyi_on_target(target_model, binary_dir, target, *otarget_data, num_threads);
    }
  }

  if (!success)
  {
    return 1;
  }

  return 0;
}
