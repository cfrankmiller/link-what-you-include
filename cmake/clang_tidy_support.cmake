# usage: clang_tidy_support()
#
# Configure clang_tidy support.
function(clang_tidy_support)
  find_program(CLANG_TIDY_COMMAND NAMES clang-tidy)
  if(CLANG_TIDY_COMMAND)
    message(STATUS "Found clang-tidy: ${CLANG_TIDY_COMMAND}")
  else()
    message(FATAL_ERROR "clang-tidy was not found")
  endif()

  set(CLANG_TIDY_CONFIG
      "${CMAKE_SOURCE_DIR}/.clang-tidy"
      CACHE PATH "Path to .clang-tidy"
    )
  mark_as_advanced(CLANG_TIDY_CONFIG)

  set(CLANG_TIDY_CHECKS_STRING ""
      CACHE STRING "String to pass to the clang-tidy --checks= argument. e.g. \"-.*,bugprone-*\"."
    )
  mark_as_advanced(CLANG_TIDY_CHECKS_STRING)

  find_program(CLANG_APPLY_REPLACEMENTS_COMMAND NAMES clang-apply-replacements)
  if(CLANG_APPLY_REPLACEMENTS_COMMAND)
    message(STATUS "Found clang-apply-replacements: ${CLANG_APPLY_REPLACEMENTS_COMMAND}")
    set(CMAKE_CXX_CLANG_TIDY_EXPORT_FIXES_DIR "${CMAKE_BINARY_DIR}/clang_tidy_fixes" PARENT_SCOPE)
    add_custom_target(
      apply_replacements
      COMMAND ${CLANG_APPLY_REPLACEMENTS_COMMAND} --style=file --style-config=${CMAKE_SOURCE_DIR}
      "${CMAKE_BINARY_DIR}/clang_tidy_fixes"
      COMMENT "Running clang-apply-replacements"
    )
  else()
    message(WARNING "clang-apply-replacements was not found, support for apply_replacements will be disabled")
  endif()

  set(options -extra-arg=-Qunused-arguments -p ${CMAKE_BINARY_DIR} --quiet)
  list(APPEND options --header-filter=${CMAKE_SOURCE_DIR})

  if(CLANG_TIDY_CONFIG)
    list(APPEND options "--config-file=${CLANG_TIDY_CONFIG}")
  endif()
  if(CLANG_TIDY_CHECKS_STRING)
    list(APPEND options --checks=${CLANG_TIDY_CHECKS_STRING})
  endif()
  if(CMAKE_COLOR_DIAGNOSTICS)
    list(APPEND options --use-color)
  endif()
  set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}" ${options} PARENT_SCOPE)
endfunction()
