// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <scanner/scan.hpp>
#include <src/scan_impl.hpp>

#include <tl/expected.hpp>

#include <string>
#include <vector>

namespace scanner
{
struct Include_data;

auto merge_includes(std::vector<tl::expected<Include_data, std::string>> include_data_array)
  -> tl::expected<Intransitive_includes, std::string>;
} // namespace scanner
