# Copyright (c) 2025 Environmental Systems Research Institute, Inc.
# SPDX-License-Identifier: Apache-2.0

set -ex

work_dir="$PWD/build/simdjson"
install_dir="$PWD/3rdparty"

if [ -d "$work_dir" ]; then
  echo "Error: this build script expects $work_dir to not exist"
  exit 1
fi
mkdir -p "$work_dir"

cd "$work_dir"
version="4.6.4"
curl -LO "https://github.com/simdjson/simdjson/archive/refs/tags/v${version}.tar.gz"
tar -xf "v${version}.tar.gz"
mv "simdjson-${version}" "$work_dir/src"

cmake -G Ninja \
      -S "$work_dir/src" -B "$work_dir/build" \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX="$install_dir"
cmake --build "$work_dir/build"
cmake --install "$work_dir/build"
