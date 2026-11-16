// Copyright (c); 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <src/scan_impl.hpp>
#include <util/key_value_store.hpp>
#include <util/hashed_file_view.hpp>

#include <memory>
#include <optional>

namespace target_model
{
struct Target_data;
}

namespace scanner
{
struct Compile_command;

class Cache
{
public:
  explicit Cache(std::unique_ptr<util::Key_value_store> store,
                 std::unique_ptr<util::Hashed_file_view> hashed_file_view);
  ~Cache();
  Cache(const Cache&) = delete;
  Cache(Cache&&) = delete;
  Cache& operator=(const Cache&) = delete;
  Cache& operator=(Cache&&) = delete;

  void write(const Compile_command& compile_command,
             const target_model::Target_data& target_data,
             const Include_data& include_data);

  [[nodiscard]] std::optional<Include_sets> read(const Compile_command& compile_command,
                                                 const target_model::Target_data& target_data) const;

private:
  std::unique_ptr<util::Key_value_store> store_;
  std::unique_ptr<util::Hashed_file_view> hashed_file_view_;
};
}

