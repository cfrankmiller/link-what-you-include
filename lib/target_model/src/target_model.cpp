// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#include <target_model/target_model.hpp>

#include <target_model/target.hpp>
#include <target_model/target_data.hpp>
#include <util/utils.hpp>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <filesystem>
#include <flat_map>
#include <functional>
#include <iterator>
#include <map>
#include <optional>
#include <unordered_set>
#include <utility>
#include <vector>

namespace target_model
{
Target_model::Target_model(std::map<Target, Target_data> target_to_target_data)
: target_to_target_data_(std::sorted_unique, // NOLINT(misc-include-cleaner) false error
                         std::make_move_iterator(target_to_target_data.begin()),
                         std::make_move_iterator(target_to_target_data.end()))
{
  for (size_t i = 0, iend = target_to_target_data_.keys().size(); i < iend; ++i)
  {
    const Target_data& target_data = target_to_target_data_.values()[i];
    for (const auto& header : target_data.interface_headers)
    {
      header_to_index_.emplace_back(header, i);
    }
    for (const auto& directory : target_data.interface_include_directories)
    {
      directory_to_index_.emplace_back(directory, i);
    }
  }
  std::ranges::sort(header_to_index_);
}

std::optional<std::reference_wrapper<const Target_data>> Target_model::get_target_data(
  const Target& target) const
{
  if (auto it = target_to_target_data_.find(target); it != target_to_target_data_.end())
  {
    return std::ref(it->second);
  }

  return {};
}

std::vector<Target> Target_model::map_header_to_targets(const std::filesystem::path& header) const
{
  std::vector<Target> result;
  auto rng = std::ranges::equal_range(header_to_index_,
                                      std::make_pair(header, 0),
                                      [](const auto& lhs, const auto& rhs)
                                      {
                                        return lhs.first < rhs.first;
                                      });
  if (!rng.empty())
  {
    for (const auto& e : rng)
    {
      result.push_back(target_to_target_data_.keys()[e.second]);
    }
    return result;
  }

  for (const auto& [directory, index] : directory_to_index_)
  {
    const auto& target_data = target_to_target_data_.values()[index];

    if (target_data.interface_include_prefixes.empty())
    {
      if (util::is_in_directory(directory, header))
      {
        result.push_back(target_to_target_data_.keys()[index]);
      }
    }
    else
    {
      for (const auto& prefix : target_data.interface_include_prefixes)
      {
        const auto prefixed_dir = std::filesystem::path{directory} /
                                  std::filesystem::path{prefix};
        if (util::is_in_directory(prefixed_dir, header))
        {
          result.push_back(target_to_target_data_.keys()[index]);
        }
      }
    }
  }

  return result;
}

void Target_model::set_interface_include_prefixes(const Target& target,
                                                  const std::set<std::string>& prefixes)
{
  if (auto it = target_to_target_data_.find(target); it != target_to_target_data_.end())
  {
    it->second.interface_include_prefixes = {prefixes.begin(), prefixes.end()};
  }
}

void Target_model::for_each_target(
  const std::function<void(const Target&, const Target_data&)>& visitor) const
{
  for (const auto& [target, data] : target_to_target_data_)
  {
    visitor(target, data);
  }
}

Target_model Target_model::create_pruned(const std::vector<Target>& targets) const
{
  std::map<Target, Target_data> pruned_target_to_target_data;

  std::vector<Target> stack = targets;
  while (!stack.empty())
  {
    auto target = stack.back();
    stack.pop_back();

    if (0 < pruned_target_to_target_data.count(target))
    {
      continue;
    }

    if (auto target_data = get_target_data(target))
    {
      pruned_target_to_target_data.emplace(target, *target_data);
      for (const auto& dep : target_data->get().dependencies)
      {
        stack.push_back(dep);
      }
    }
  }

  return Target_model{std::move(pruned_target_to_target_data)};
}

} // namespace target_model
