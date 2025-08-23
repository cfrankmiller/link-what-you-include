// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <tidy/config.hpp>

#include <tl/expected.hpp>

#include <filesystem>
#include <string>

namespace tidy
{
auto load_config(std::filesystem::path config_path) -> tl::expected<Config, std::string>;
}
