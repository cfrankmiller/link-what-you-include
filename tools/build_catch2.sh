# Copyright (c) 2025 Environmental Systems Research Institute, Inc.
# SPDX-License-Identifier: Apache-2.0

set -ex

root_dir="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )"/.. &> /dev/null && pwd )"
work_dir="$root_dir/build/catch2"
install_dir="$root_dir/3rdparty"

mkdir -p "$work_dir"
cd "$work_dir"

if [ ! -d "$work_dir/src" ]; then
  version="3.15.1"
  curl -LO "https://github.com/catchorg/Catch2/archive/refs/tags/v${version}.tar.gz"
  tar -xf "v${version}.tar.gz"
  mv "Catch2-${version}" "$work_dir/src"
fi

cmake --fresh \
      -G Ninja \
      -S "$work_dir/src" -B "$work_dir/build" \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX="$install_dir" \
      -DCMAKE_CXX_STANDARD="23"
cmake --build "$work_dir/build"
cmake --install "$work_dir/build"
