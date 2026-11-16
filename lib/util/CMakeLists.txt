# Copyright (c) 2025 Environmental Systems Research Institute, Inc.
# SPDX-License-Identifier: Apache-2.0

add_library(lib_util)
target_sources(lib_util
  PUBLIC FILE_SET HEADERS BASE_DIRS include FILES
    include/util/arg_parser.hpp
    include/util/byte_hasher.hpp
    include/util/byte_reader.hpp
    include/util/byte_writer.hpp
    include/util/byte_visitor.hpp
    include/util/byte_visitor/cstdint.hpp
    include/util/byte_visitor/filesystem.hpp
    include/util/byte_visitor/set.hpp
    include/util/byte_visitor/string.hpp
    include/util/byte_visitor/vector.hpp
    include/util/hash.hpp
    include/util/hashed_file_view.hpp
    include/util/key_value_store.hpp
    include/util/parallel_transformer.hpp
    include/util/scope.hpp
    include/util/utils.hpp
  PRIVATE
    src/byte_hasher.cpp
    src/hash.cpp
    src/hashed_file_view.cpp
    src/key_value_store.cpp
    src/parallel_transformer.cpp
    src/utils.cpp
  )
target_link_libraries(lib_util PRIVATE xxHash::xxhash)

if(BUILD_TESTING)
  add_executable(lib_util_test)
  target_sources(lib_util_test
    PRIVATE
      test/arg_parser_test.cpp
      test/byte_hasher_test.cpp
      test/byte_reader_writer_test.cpp
      test/hashed_file_view_test.cpp
      test/hash_test.cpp
      test/key_value_store_test.cpp
      test/parallel_transformer_test.cpp
      test/scope_test.cpp
      test/utils_test.cpp
    )
  target_link_libraries(lib_util_test
    PRIVATE
      lib_util
      Catch2::Catch2WithMain
    )
  catch_discover_tests(lib_util_test ADD_TAGS_AS_LABELS)
endif()
