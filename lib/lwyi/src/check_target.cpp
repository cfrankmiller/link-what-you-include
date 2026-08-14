// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#include <lwyi/check_target.hpp>

#include <lwyi/config.hpp>
#include <lwyi/dependency_scope.hpp>
#include <lwyi/dependency_visibility.hpp>
#include <scanner/include.hpp>
#include <scanner/scan.hpp>
#include <target_model/target.hpp>
#include <target_model/target_data.hpp>
#include <target_model/target_model.hpp>

#include <cassert>
#include <map>
#include <optional>
#include <utility>
#include <vector>

namespace lwyi
{
std::vector<LWYI_error> check_target(const lwyi::Config& config,
                                     const target_model::Target_model& target_model,
                                     const target_model::Target& target,
                                     const target_model::Target_data& target_data,
                                     const scanner::Intransitive_includes& target_includes)
{
  std::map<target_model::Target, Dependency_visibility> visibility_map;

  // only include linked dependencies that are targets in the target model
  for (const auto& dep : target_data.interface_dependencies)
  {
    // TODO Produce an error if dep is not ignored and shares includes or include directories with other targets
    if (target_model.get_target_data(dep).has_value())
    {
      visibility_map[dep].link_scope |= Dependency_scope::interface_scope;
    }
  }
  for (const auto& dep : target_data.dependencies)
  {
    // TODO Produce an error if dep is not ignored and shares includes or include directories with other targets
    if (target_model.get_target_data(dep).has_value())
    {
      visibility_map[dep].link_scope |= Dependency_scope::private_scope;
    }
  }

  // only include header dependencies that map to targets in the target model
  std::map<target_model::Target, std::vector<scanner::Include>> included_interface_deps_map;
  for (const auto& include : target_includes.interface_includes)
  {
    auto targets = target_model.map_header_to_targets(include.path);
    // TODO: Produce an error if there is more than one target here after removing those that should be ignored
    for (const auto& target : targets)
    {
      visibility_map[target].include_scope |= Dependency_scope::interface_scope;
      included_interface_deps_map[target].push_back(include);
    }
  }
  std::map<target_model::Target, std::vector<scanner::Include>> included_deps_map;
  for (const auto& include : target_includes.includes)
  {
    auto targets = target_model.map_header_to_targets(include.path);
    // TODO: Produce an error if there is more than one target here after removing those that should be ignored
    for (const auto& target : targets)
    {
      visibility_map[target].include_scope |= Dependency_scope::private_scope;
      included_deps_map[target].push_back(include);
    }
  }

  const auto& target_config = config.get_target_config(target);

  std::vector<LWYI_error> errors;
  for (const auto& [dep, visibility] : visibility_map)
  {
    const auto& dep_config = config.get_target_config(dep);
    bool allow_over_include = target_config.allow_includes_set.contains(dep) ||
                              target_config.allow_includes ||
                              dep_config.interface_allow_includes;
    bool allow_over_link = target_config.allow_links_set.contains(dep) ||
                           target_config.allow_links || dep_config.interface_allow_links;

    const auto visibility_class = compute_dependency_visibility_class(visibility);
    if ((visibility_class == Dependency_visibility_class::good) ||
        (allow_over_include && visibility_class == Dependency_visibility_class::over_include) ||
        (allow_over_link && visibility_class == Dependency_visibility_class::over_link) ||
        (allow_over_include && allow_over_link &&
         visibility_class == Dependency_visibility_class::bad))
    {
      continue;
    }

    std::optional<target_model::Source_location> linked_location;
    if (auto it = target_data.dependency_locations.find(dep);
        it != target_data.dependency_locations.end())
    {
      linked_location = it->second;
    }

    LWYI_error error{dep,
                     visibility.link_scope,
                     visibility.include_scope,
                     std::move(linked_location),
                     {}};
    if (!!(visibility.include_scope & Dependency_scope::interface_scope))
    {
      auto it = included_interface_deps_map.find(dep);
      assert(it != included_interface_deps_map.end());
      error.sample_includes = it->second;
    }
    if (!!(visibility.include_scope & Dependency_scope::private_scope))
    {
      auto it = included_deps_map.find(dep);
      assert(it != included_deps_map.end());
      error.sample_includes.insert(error.sample_includes.end(),
                                   it->second.begin(),
                                   it->second.end());
    }
    errors.push_back(std::move(error));
  }

  return errors;
}
} // namespace lwyi
