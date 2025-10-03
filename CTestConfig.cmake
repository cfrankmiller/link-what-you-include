# Copyright (c) 2025 Environmental Systems Research Institute, Inc.
# SPDX-License-Identifier: Apache-2.0

set(CTEST_PROJECT_NAME link-what-you-include)
set(CTEST_NIGHTLY_START_TIME 01:00:00 America/Los_Angeles)

if(CMAKE_VERSION VERSION_GREATER 3.14)
  set(CTEST_SUBMIT_URL https://my.cdash.org/submit.php?project=link-what-you-include)
else()
  set(CTEST_DROP_METHOD "https")
  set(CTEST_DROP_SITE "my.cdash.org")
  set(CTEST_DROP_LOCATION "/submit.php?project=link-what-you-include")
endif()

set(CTEST_DROP_SITE_CDASH TRUE)
