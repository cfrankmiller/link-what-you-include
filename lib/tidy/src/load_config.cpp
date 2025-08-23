#include <tidy/load_config.hpp>

#include <src/load_config_impl.hpp>
#include <tidy/config.hpp>

#include <fmt/format.h>
#include <simdjson.h>
#include <tl/expected.hpp>

#include <filesystem>
#include <string>

namespace tidy
{
auto load_config(std::filesystem::path config_path) -> tl::expected<Config, std::string>
{
  simdjson::padded_string raw_config;
  if (auto error = simdjson::padded_string::load(config_path.string()).get(raw_config))
  {
    return tl::unexpected(fmt::format("Failed to load config file {}: {}\n",
                                      config_path.string(),
                                      simdjson::error_message(error)));
  }

  return load_config_impl(raw_config)
    .map_error(
      [&](const std::string& err)
      {
        return fmt::format("Failed to load config file {}: {}\n", config_path.string(), err);
      });
}
} // namespace tidy
