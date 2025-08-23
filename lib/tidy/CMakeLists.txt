# Copyright (c) 2025 Environmental Systems Research Institute, Inc.
# SPDX-License-Identifier: Apache-2.0

add_library(lib_tidy)
target_sources(lib_tidy
  PUBLIC FILE_SET HEADERS BASE_DIRS include FILES
    include/tidy/config.hpp
    include/tidy/tidy.hpp
    include/tidy/load_config.hpp
  PRIVATE
    src/tidy.cpp
    src/cluster_diff.hpp
    src/cluster_diff.cpp
    src/load_config.cpp
    src/load_config_impl.hpp
    src/load_config_impl.cpp
  )
target_link_libraries(lib_tidy
  PUBLIC
    lib_target_model
    tl::expected
  PRIVATE
    lib_lwyi
    fmt::fmt
    simdjson::simdjson
  )
target_include_directories(lib_tidy
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  )

if(BUILD_TESTING)
  add_executable(lib_tidy_test
    test/cluster_diff_test.cpp
    test/tidy_test.cpp
    test/load_config_impl_test.cpp
    )
  target_include_directories(lib_tidy_test
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    )
  target_link_libraries(lib_tidy_test
    PRIVATE
      lib_tidy
      lib_target_model
      simdjson::simdjson
      Catch2::Catch2WithMain
    )
  catch_discover_tests(lib_tidy_test ADD_TAGS_AS_LABELS)
endif()

if(ENABLE_DOGFOODING)
  link_what_you_include(lib_tidy)
  link_what_you_include(lib_tidy_test)
endif()
