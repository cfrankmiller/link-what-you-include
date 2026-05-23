# Copyright (c) 2025 Environmental Systems Research Institute, Inc.
# SPDX-License-Identifier: Apache-2.0

set(CTEST_SITE github.esri.lwyi)
set(CTEST_USE_LAUNCHERS YES)

find_package(Git)
set(CTEST_UPDATE_VERSION_ONLY ON)
set(CTEST_UPDATE_COMMAND "${GIT_EXECUTABLE}")

set(CTEST_CMAKE_GENERATOR Ninja)
