// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#include <cli/parse_arguments.hpp>
#include <message/message.hpp>
#include <src/run_lwyi.hpp>

#include <expected>
#include <string>

cli::Command_options configure_output(cli::Command_options options)
{
  message::configure(options.color_output, options.message_level);
  return options;
}

std::expected<int, std::string> print_error(const std::string& error)
{
  message::print(error, message::Style::error);
  return 1;
}

int main(int argc, const char* argv[])
{
  return cli::parse_arguments(argc, argv)
    .transform(configure_output)
    .and_then(run_lwyi)
    .or_else(print_error)
    .value_or(1);
}
