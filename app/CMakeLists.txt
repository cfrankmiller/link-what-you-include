# Copyright (c) 2025 Environmental Systems Research Institute, Inc.
# SPDX-License-Identifier: Apache-2.0

add_executable(lwyi)
target_sources(lwyi PRIVATE
  src/tidy_tool.hpp
  src/tidy_tool.cpp
  src/graph_tool.hpp
  src/graph_tool.cpp
  src/main.cpp
  src/run_lwyi.hpp
  src/run_lwyi.cpp
  src/run_lwyi_on_target.hpp
  src/run_lwyi_on_target.cpp
  src/run_tool.hpp
  src/run_tool.cpp
  )
target_include_directories(lwyi PRIVATE .)
target_link_libraries(
  lwyi
  PRIVATE 
    lib_lwyi
    lib_tidy
    lib_scanner
    lib_target_model
    lib_util
    fmt::fmt
    simdjson::simdjson
    tl::expected
  )

install(TARGETS lwyi)

if(ENABLE_DOGFOODING)
  link_what_you_include(lwyi)
  add_test(NAME "dogfood test" COMMAND lwyi -d ${CMAKE_BINARY_DIR})
endif()
