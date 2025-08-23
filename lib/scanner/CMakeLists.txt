# Copyright (c) 2025 Environmental Systems Research Institute, Inc.
# SPDX-License-Identifier: Apache-2.0

add_library(lib_scanner)
target_sources(lib_scanner
  PUBLIC FILE_SET HEADERS BASE_DIRS include FILES
    include/scanner/include.hpp
    include/scanner/scan.hpp
  PRIVATE
    src/merge_includes.hpp
    src/merge_includes.cpp
    src/scan.cpp
    src/scan_impl.hpp
    src/scan_impl.cpp
  )
target_include_directories(lib_scanner
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  )
target_link_libraries(lib_scanner
  PUBLIC
    tl::expected
  PRIVATE
    lib_util
    lib_target_model
    fmt::fmt
    clangDependencyScanning
    clangTooling
  )

# get resource directory
get_target_property(clang_exe clang LOCATION)
execute_process(
  COMMAND ${clang_exe} -print-resource-dir
  OUTPUT_VARIABLE clang_resource_dir)
string(STRIP ${clang_resource_dir} clang_resource_dir)
cmake_path(SET clang_resource_dir ${clang_resource_dir})
target_compile_definitions(lib_scanner
  PRIVATE "LWYI_CLANG_RESOURCE_DIR=${clang_resource_dir}"
  )
message(STATUS "using clang: ${clang_exe}")
message(STATUS "clang_resource_dir: \"${clang_resource_dir}\"")

if(BUILD_TESTING)
  add_executable(lib_scanner_test
    test/scan_test.cpp
    )
  target_include_directories(lib_scanner_test
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    )
  target_link_libraries(lib_scanner_test
    PRIVATE
      lib_scanner
      lib_target_model
      fmt::fmt
      Catch2::Catch2WithMain
      clangTooling
    )
  catch_discover_tests(lib_scanner_test ADD_TAGS_AS_LABELS)
endif()

if(ENABLE_DOGFOODING)
  link_what_you_include(lib_scanner)
  link_what_you_include(lib_scanner_test)
endif()

