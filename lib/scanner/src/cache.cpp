// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#include <src/cache.hpp>

#include <src/byte_visitors.hpp>
#include <src/compile_command.hpp>
#include <target_model/target_data.hpp>
#include <util/byte_hasher.hpp>
#include <util/byte_reader.hpp>
#include <util/byte_writer.hpp>
#include <util/hash.hpp>

// There are two types of items cached.
//
// 1. Transitive includes cache
//    - Key: 128-bit combined hash of:
//      - command line hash
//      - main file content hash
//    - Value:
//      - set of transitivly included non-system file paths relative to main file
//
// 2. Intransitive includes cache
//    - Key: 128-bit combined hash of:
//      - command line hash
//      - main file content hash
//      - Target_data hash. (This feels too strong but good enough for now)
//      - The combined hash of the hash of the content of each file in the transitive includes 
//    - Value
//      - Intransitive includes. The result of the scan.


namespace scanner
{
Cache::Cache(std::unique_ptr<util::Key_value_store> store,
             std::unique_ptr<util::Hashed_file_view> hashed_file_view)
: store_(std::move(store)),
  hashed_file_view_(std::move(hashed_file_view))
{
}

Cache::~Cache() = default;

void Cache::write(const Compile_command& compile_command,
                  const target_model::Target_data& target_data,
                  const Include_data& include_data)
{
  const auto main_file_hash = hashed_file_view_->hash_of(compile_command.source);
  if (!main_file_hash)
  {
    return;
  }

  util::Byte_hasher hasher;
  hasher(compile_command);
  hasher(*main_file_hash);

  const auto transitive_includes_key = hasher.digest();

  hasher(target_data);
  hasher(include_data.transitive_includes_hash);

  const auto intransitive_includes_key = hasher.digest();

  util::Byte_writer serializer;
  serializer(include_data.transitive_includes);
  std::ignore = store_->write(transitive_includes_key, serializer.view_buffer());

  serializer.clear();
  serializer(include_data.include_sets);
  std::ignore = store_->write(intransitive_includes_key, serializer.view_buffer());
}

std::optional<Include_sets> Cache::read(const Compile_command& compile_command,
                                        const target_model::Target_data& target_data) const
{
  const auto main_file_hash = hashed_file_view_->hash_of(compile_command.source);
  if (!main_file_hash)
  {
    return std::nullopt;
  }

  util::Byte_hasher hasher;
  hasher(compile_command);
  hasher(*main_file_hash);

  const auto transitive_includes_key = hasher.digest();

  auto transitive_includes_read_result = store_->read(transitive_includes_key);
  if (!transitive_includes_read_result || transitive_includes_read_result.value().empty())
  {
    return std::nullopt;
  }

  std::vector<std::filesystem::path> transitive_includes;
  util::Byte_reader transitive_includes_deserializer(transitive_includes_read_result.value());
  transitive_includes_deserializer(transitive_includes);
  if (!transitive_includes_deserializer)
  {
    return std::nullopt;
  }

  util::Byte_hasher transitive_includes_hasher;
  for (const auto& relative_include : transitive_includes)
  {
    const auto file = (compile_command.source.parent_path() / relative_include).lexically_normal();
    const auto file_hash = hashed_file_view_->hash_of(file);
    if (!file_hash)
    {
      return std::nullopt;
    }
    transitive_includes_hasher(*file_hash);
  }

  hasher(target_data);
  hasher(transitive_includes_hasher.digest());

  const auto intransitive_includes_key = hasher.digest();
  auto intransitive_includes_read_result = store_->read(intransitive_includes_key);
  if (!intransitive_includes_read_result)
  {
    return std::nullopt;
  }

  Include_sets include_sets;
  util::Byte_reader intransitive_includes_deserializer(intransitive_includes_read_result.value());
  intransitive_includes_deserializer(include_sets);
  if (!intransitive_includes_deserializer)
  {
    return std::nullopt;
  }

  return include_sets;
}

} // namespace scanner
