// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#include <lwyi/parse_arguments.hpp>

#include <lwyi/command_options.hpp>
#include <util/arg_parser.hpp>

#include <fmt/format.h>
#include <tl/expected.hpp>

#include <cassert>
#include <cstdint>
#include <iterator>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace lwyi
{
namespace
{
constexpr std::string_view usage_string = R"(Usage:
  {0} [options]

Possible options:
  -h, --help                Print this help message.

  -d, --binary_dir DIR      Path to the directory with input files. Default is
                            the current directory.
  -t, --targets TARGETS...  Limit analysis to the given targets.
  -j, --parallel COUNT      Number of threads used to process source files.
                            Default depends on system.

  --tool TOOL [OPTIONS...]  Run a tool. All subsequent arguments are passed to
                            the tool. This is undocumented and serves as a place
                            holder for future features.)";

struct Options
{
  bool help{false};
  std::string_view binary_dir;
  uint32_t num_threads{0};
  std::vector<std::string_view> targets;
  std::vector<std::string_view> sources;
  std::vector<std::string_view> tool_command;
};

constexpr auto parser = util::arg_parser<Options>()
                          .arg("-h", "--help", &Options::help)
                          .arg("-d", "--binary_dir", &Options::binary_dir)
                          .arg("-t", "--targets", &Options::targets)
                          .arg("-j", "--parallel", &Options::num_threads)
                          .terminal_arg("--tool", &Options::tool_command);

auto usage(std::string_view name) -> std::string
{
  return fmt::format(usage_string, name);
}
} // namespace

auto parse_arguments(int argc, const char* const* argv)
  -> tl::expected<Command_options, std::string>
{
  assert(0 < argc);
  assert(argv != nullptr);

  const std::string_view name = *argv;

  auto result = parser.parse(std::next(argv, 1), std::next(argv, argc));

  if (!result.has_value())
  {
    return tl::unexpected(fmt::format("{}\n{}\n", result.error(), usage(name)));
  }
  if (result.value().help)
  {
    return tl::unexpected(usage(name));
  }

  auto& options = result.value();
  return Command_options{options.binary_dir,
                         std::move(options.targets),
                         std::move(options.tool_command),
                         options.num_threads};
}
} // namespace lwyi
