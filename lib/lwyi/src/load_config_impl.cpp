// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#include <src/load_config_impl.hpp>

#include <lwyi/config.hpp>
#include <target_model/target.hpp>

#include <simdjson.h>

#include <cassert>
#include <expected>
#include <format>
#include <map>
#include <string>
#include <string_view>
#include <utility>

namespace lwyi
{
std::expected<Config, std::string> load_config_impl(const simdjson::padded_string& raw_config)
{
  simdjson::ondemand::parser parser;
  simdjson::ondemand::document doc;
  if (auto error = parser.iterate(raw_config).get(doc))
  {
    return std::unexpected(std::format("root document: {}", simdjson::error_message(error)));
  }

  std::map<target_model::Target, Target_config> target_configs;

  simdjson::ondemand::object root;
  if (auto error = doc.get_object().get(root))
  {
    return std::unexpected(std::format("root object: {}", simdjson::error_message(error)));
  }

  simdjson::ondemand::value targets_value;
  if (auto error = root.find_field_unordered("targets").get(targets_value))
  {
    if (error == simdjson::NO_SUCH_FIELD)
    {
      return {};
    }
    return std::unexpected(
      std::format("field 'targets': {}", simdjson::error_message(error)));
  }

  simdjson::ondemand::object targets;
  if (auto error = targets_value.get_object().get(targets))
  {
    return std::unexpected(
      std::format("field 'targets': {}", simdjson::error_message(error)));
  }

  for (auto field : targets)
  {
    std::string_view target_name;
    if (auto error = field.unescaped_key().get(target_name))
    {
      return std::unexpected(std::format("target name: {}", simdjson::error_message(error)));
    }

    simdjson::ondemand::object target_object;
    if (auto error = field.value().get_object().get(target_object))
    {
      return std::unexpected(
        std::format("target '{}': {}", target_name, simdjson::error_message(error)));
    }

    Target_config target_config;

    simdjson::ondemand::value prefixes_value;
    if (!target_object.find_field_unordered("interface_include_prefixes").get(prefixes_value))
    {
      simdjson::ondemand::array prefixes;
      if (auto error = prefixes_value.get_array().get(prefixes))
      {
        return std::unexpected(std::format("target '{}'.interface_include_prefixes: {}",
                                           target_name,
                                           simdjson::error_message(error)));
      }
      for (auto prefix_value : prefixes)
      {
        std::string_view prefix;
        if (auto error = prefix_value.get(prefix))
        {
          return std::unexpected(std::format("target '{}'.interface_include_prefixes[]: {}",
                                             target_name,
                                             simdjson::error_message(error)));
        }
        target_config.interface_include_prefixes.insert(std::string(prefix));
      }
    }

    simdjson::ondemand::value allow_includes_value;
    if (!target_object.find_field_unordered("allow_includes").get(allow_includes_value))
    {
      simdjson::ondemand::json_type type = allow_includes_value.type();
      if (type == simdjson::ondemand::json_type::boolean)
      {
        bool allow_includes = false;
        [[maybe_unused]] auto error = allow_includes_value.get_bool().get(allow_includes);
        assert(!error);
        target_config.allow_includes = allow_includes;
      }
      else if (type == simdjson::ondemand::json_type::array)
      {
        simdjson::ondemand::array allow_includes;
        [[maybe_unused]] auto error = allow_includes_value.get_array().get(allow_includes);
        assert(!error);
        for (auto target_value : allow_includes)
        {
          std::string_view target;
          if (auto error = target_value.get(target))
          {
            return std::unexpected(std::format("target '{}'.allow_includes[]: {}",
                                               target_name,
                                               simdjson::error_message(error)));
          }
          target_config.allow_includes_set.emplace(std::string(target));
        }
      }
      else
      {
        return std::unexpected(
          std::format("target '{}'.allow_includes: expect a bool or an array", target_name));
      }
    }

    simdjson::ondemand::value allow_links_value;
    if (!target_object.find_field_unordered("allow_links").get(allow_links_value))
    {
      simdjson::ondemand::json_type type = allow_links_value.type();
      if (type == simdjson::ondemand::json_type::boolean)
      {
        bool allow_links = false;
        [[maybe_unused]] auto error = allow_links_value.get_bool().get(allow_links);
        assert(!error);
        target_config.allow_links = allow_links;
      }
      else if (type == simdjson::ondemand::json_type::array)
      {
        simdjson::ondemand::array allow_links;
        [[maybe_unused]] auto error = allow_links_value.get_array().get(allow_links);
        assert(!error);
        for (auto target_value : allow_links)
        {
          std::string_view target;
          if (auto error = target_value.get(target))
          {
            return std::unexpected(std::format("target '{}'.allow_links[]: {}",
                                               target_name,
                                               simdjson::error_message(error)));
          }
          target_config.allow_links_set.emplace(std::string(target));
        }
      }
      else
      {
        return std::unexpected(
          std::format("target '{}'.allow_links: expect a bool or an array", target_name));
      }
    }

    simdjson::ondemand::value interface_allow_includes_value;
    if (!target_object.find_field_unordered("interface_allow_includes")
           .get(interface_allow_includes_value))
    {
      bool interface_allow_includes = false;
      if (auto error = interface_allow_includes_value.get_bool().get(interface_allow_includes))
      {
        return std::unexpected(std::format("target '{}'.interface_allow_includes: {}",
                                           target_name,
                                           simdjson::error_message(error)));
      }
      target_config.interface_allow_includes = interface_allow_includes;
    }

    simdjson::ondemand::value interface_allow_links_value;
    if (!target_object.find_field_unordered("interface_allow_links").get(interface_allow_links_value))
    {
      bool interface_allow_links = false;
      if (auto error = interface_allow_links_value.get_bool().get(interface_allow_links))
      {
        return std::unexpected(std::format("target '{}'.interface_allow_links: {}",
                                           target_name,
                                           simdjson::error_message(error)));
      }
      target_config.interface_allow_links = interface_allow_links;
    }

    target_configs.emplace(target_model::Target{std::string(target_name)},
                           std::move(target_config));
  }

  return Config{std::move(target_configs)};
}
} // namespace lwyi
