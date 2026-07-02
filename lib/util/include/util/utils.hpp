// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <filesystem>

namespace util
{
auto is_in_directory(std::filesystem::path dir, const std::filesystem::path& file)
  -> bool;
}
