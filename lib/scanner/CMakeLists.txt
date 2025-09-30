# Copyright (c) 2025 Environmental Systems Research Institute, Inc.
# SPDX-License-Identifier: Apache-2.0

add_library(lib_scanner)
target_sources(lib_scanner
  PUBLIC FILE_SET HEADERS BASE_DIRS include FILES
    include/scanner/include.hpp
    include/scanner/scan.hpp
  PRIVATE
    src/executable_path.hpp
    src/executable_path.cpp
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

# copy/install the clang resource directory into our build/install trees
get_target_property(clang_exe clang LOCATION)
message(STATUS "Using clang: ${clang_exe}")

execute_process(
  COMMAND ${clang_exe} -print-resource-dir
  OUTPUT_VARIABLE clang_resource_dir)
string(STRIP ${clang_resource_dir} clang_resource_dir)
cmake_path(SET clang_resource_dir ${clang_resource_dir})
if(NOT EXISTS ${clang_resource_dir}/include)
  message(FATAL_ERROR "Clang resource directory not found")
endif()
message(STATUS "Found clang resource directory: \"${clang_resource_dir}\"")

message(STATUS "Copying clang resource include directory to  \"${CMAKE_BINARY_DIR}/${LWYI_install_resource_dir}\"")
file(COPY ${clang_resource_dir}/include DESTINATION ${CMAKE_BINARY_DIR}/${LWYI_install_resource_dir})
install(DIRECTORY ${clang_resource_dir}/include DESTINATION ${LWYI_install_resource_dir})

set(relative_resource_dir ${CMAKE_BINARY_DIR}/${LWYI_install_resource_dir})
cmake_path(RELATIVE_PATH relative_resource_dir BASE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
message(STATUS "Setting relative path to resource directory to \"${relative_resource_dir}")
target_compile_definitions(lib_scanner
  PRIVATE "LWYI_RELATIVE_RESOURCE_DIR=${relative_resource_dir}"
  )

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

