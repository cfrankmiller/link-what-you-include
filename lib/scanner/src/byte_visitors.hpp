// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#include <scanner/include.hpp>
#include <src/compile_command.hpp>
#include <src/include_sets.hpp>
#include <target_model/target_data.hpp>
#include <util/byte_hasher.hpp>
#include <util/byte_visitor.hpp>
#include <util/byte_visitor/cstdint.hpp>
#include <util/byte_visitor/filesystem.hpp>
#include <util/byte_visitor/set.hpp>
#include <util/byte_visitor/string.hpp>
#include <util/byte_visitor/unordered_set.hpp>
#include <util/byte_visitor/vector.hpp>

#include <utility>

namespace scanner
{
template <class T>
  requires std::same_as<std::remove_cvref_t<T>, Source_line>
void visit(util::Byte_visitor auto& v, T&& source_line)
{
  v(std::forward_like<T>(source_line.source));
  v(std::forward_like<T>(source_line.line));
}

template <class T>
  requires std::same_as<std::remove_cvref_t<T>, Include>
void visit(util::Byte_visitor auto& v, T&& include)
{
  v(std::forward_like<T>(include.path));
  v(std::forward_like<T>(include.include_chain));
}

template <class T>
  requires std::same_as<std::remove_cvref_t<T>, Include_sets>
void visit(util::Byte_visitor auto& v, T&& include_sets)
{
  v(std::forward_like<T>(include_sets.includes));
  v(std::forward_like<T>(include_sets.interface_includes));
}

inline void visit(util::Byte_hasher& h, const Compile_command& compile_command)
{
  h(compile_command.cwd);
  h(compile_command.source);
  h(compile_command.command);
}
} // namespace scanner


namespace target_model
{
inline void visit(util::Byte_hasher& h, const Target& target)
{
  h(target.name);
}

inline void visit(util::Byte_hasher& h, const Target_data& target_data)
{
  h(target_data.imported);
  h(target_data.interface_headers);
  h(target_data.interface_include_directories);
  h(target_data.interface_include_prefixes);
  h(target_data.interface_dependencies);
  h(target_data.dependencies);
  h(target_data.sources);
  h(target_data.headers);
  h(target_data.verify_interface_header_sets_sources);
}
} // namespace target_model
