// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <target_model/target.hpp>

#include <map>
#include <set>
#include <vector>

namespace tidy
{
struct Config
{
  std::map<target_model::Target, std::set<target_model::Target>> forbidden_dependencies;
  std::vector<std::set<target_model::Target>> allowed_clusters;
};
} // namespace tidy
