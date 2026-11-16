// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#include <util/key_value_store.hpp>

#include <util/hash.hpp>
#include <util/scope.hpp>

#include <bit>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <memory>
#include <random>
#include <string>
#include <system_error>
#include <utility>

namespace util
{
namespace
{
// Binary file format:
//   0 - 3     Magic bytes (LWYI)
//   4 - 7     32-bit little-endian integer version (0)
//   8 - N     payload
//   N - N+16  128-bit hash of payload

constexpr std::array<std::byte, 4> magic_{std::byte{'L'},
                                          std::byte{'W'},
                                          std::byte{'Y'},
                                          std::byte{'I'}};
constexpr uint32_t version_{0};

std::string hex_string_(Hash_value hash)
{
  std::string result;
  result.reserve(hash.size() * 2);
  for (auto byte : hash)
  {
    result += std::format("{:02x}", std::to_integer<uint8_t>(byte));
  }
  return result;
}

template <typename T>
  requires(std::is_integral_v<T>)
void write_(std::ofstream& os, T value)
{
  if constexpr (std::endian::native == std::endian::big)
  {
    value = std::byteswap(value);
  }
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  os.write(reinterpret_cast<const char*>(&value),
           static_cast<std::streamsize>(sizeof(value)));
}

void write_(std::ofstream& os, std::span<const std::byte> value)
{
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  os.write(reinterpret_cast<const char*>(value.data()),
           static_cast<std::streamsize>(value.size()));
}

template <typename T>
  requires(std::is_integral_v<T>)
void read_(std::ifstream& os, T& value)
{
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  os.read(reinterpret_cast<char*>(&value), static_cast<std::streamsize>(sizeof(value)));
  if constexpr (std::endian::native == std::endian::big)
  {
    value = std::byteswap(value);
  }
}

void read_(std::ifstream& os, std::span<std::byte> value)
{
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  os.read(reinterpret_cast<char*>(value.data()), static_cast<std::streamsize>(value.size()));
}
}

class File_store : public Key_value_store
{
public:
  explicit File_store(std::filesystem::path cache_dir)
  : cache_dir_(std::move(cache_dir))
  {
  }

  std::expected<void, std::string> write(Hash_value key, std::span<const std::byte> value) override
  {
    const std::string key_str = hex_string_(key);
    std::filesystem::path dest_dir = cache_dir_ / key_str.substr(0, 2);
    std::filesystem::path dest_file = key_str.substr(2);
    std::filesystem::path dest_path = dest_dir / dest_file;

    if (!std::filesystem::exists(dest_dir))
    {
      std::error_code ec;
      std::filesystem::create_directories(dest_dir, ec);
      if (ec)
      {
        return std::unexpected(
          std::format("Failed to create directory {}: {}", dest_dir.string(), ec.message()));
      }
    }

    if (std::filesystem::exists(dest_path))
    {
      // Not an error.
      return {};
    }

    std::filesystem::path temp_path =
      dest_dir / std::format("{}-{:08x}.tmp", dest_file.string(), random_int());
    util::scope_exit on_exit(
      [temp_path]()
      {
        std::error_code ec;
        std::filesystem::remove(temp_path, ec);
      });

    std::ofstream ofs(temp_path, std::ios::binary | std::ios::noreplace);
    if (!ofs)
    {
      return std::unexpected(
        std::format("Failed to write {}. Open tmp file failed.\n", key_str));
    }

    write_(ofs, magic_);
    write_(ofs, version_);
    write_(ofs, value);
    write_(ofs, hash(value));

    ofs.close();
    if (!ofs)
    {
      return std::unexpected(
        std::format("Failed to write {}. Write to tmp file failed.\n", key_str));
    }

    std::error_code ec;
    std::filesystem::rename(temp_path, dest_path, ec);
    if (ec)
    {
      return std::unexpected(
        std::format("Failed to write {}. Rename tmp file failed.\n", key_str));
    }

    return {};
  }

  [[nodiscard]] std::expected<std::vector<std::byte>, std::string> read(Hash_value key) const override
  {
    const std::string key_str = hex_string_(key);
    std::filesystem::path path = cache_dir_ / key_str.substr(0, 2) / key_str.substr(2);

    std::ifstream ifs(path, std::ios::binary | std::ios::ate);
    if (!ifs.is_open() || ifs.fail())
    {
      // Non-existence is not an error.
      return {};
    }

    const auto file_size = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    std::array<std::byte, 4> magic{};
    uint32_t version{};
    util::Hash_value payload_hash{};
    const auto header_footer_size = sizeof(magic) + sizeof(version) + sizeof(payload_hash);

    if (file_size < static_cast<std::streamsize>(header_footer_size))
    {
      return std::unexpected(std::format("Failed to read {}. Inadequate file size.\n", key_str));
    }

    read_(ifs, magic);
    read_(ifs, version);

    if (!ifs || magic != magic_ || version != version_)
    {
      return std::unexpected(std::format("Failed to read {}. Invalid content.\n", key_str));
    }

    const auto payload_size = static_cast<uint64_t>(file_size) - header_footer_size;
    std::vector<std::byte> payload(payload_size);
    read_(ifs, payload);
    read_(ifs, payload_hash);

    if (!ifs)
    {
      return std::unexpected(std::format("Failed to read {}.\n", key_str));
    }

    if (hash(payload.data(), payload.size()) != payload_hash)
    {
      return std::unexpected(std::format("Failed to read {}. Corrupted file.\n", key_str));
    }

    return payload;
  }

private:
  uint32_t random_int()
  {
    static thread_local std::mt19937 gen(std::random_device{}());
    return std::uniform_int_distribution<uint32_t>{}(gen);
  }

  std::filesystem::path cache_dir_;
};

std::unique_ptr<Key_value_store> default_key_value_store(std::filesystem::path cache_dir)
{
  return std::make_unique<File_store>(std::move(cache_dir));
}
} // namespace util
