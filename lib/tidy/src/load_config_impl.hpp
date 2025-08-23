// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <tidy/config.hpp>

#include <simdjson.h>
#include <tl/expected.hpp>

#include <string>

namespace tidy
{
auto load_config_impl(const simdjson::padded_string& raw_config)
  -> tl::expected<Config, std::string>;
}
