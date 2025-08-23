// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <tl/expected.hpp>

#include <filesystem>
#include <string>

namespace target_model
{
class Target_model;

class Target_model_loader
{
public:
  static auto create() -> std::unique_ptr<Target_model_loader>;

  virtual ~Target_model_loader() = default;

  virtual auto load_json(const std::filesystem::path& path)
    -> tl::expected<void, std::string> = 0;

  virtual auto make_target_model() -> Target_model = 0;
};

} // namespace target_model
