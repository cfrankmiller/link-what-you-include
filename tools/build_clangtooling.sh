# Copyright (c) 2025 Environmental Systems Research Institute, Inc.
# SPDX-License-Identifier: Apache-2.0

set -ex

work_dir="$PWD/build/llvm"
install_dir="$PWD/3rdparty"

if [ -d "$work_dir" ]; then
  echo "Error: this build script expects $work_dir to not exist"
  exit 1
fi
mkdir -p "$work_dir"

cd "$work_dir"

version="22.1.7"
curl -LO "https://github.com/llvm/llvm-project/releases/download/llvmorg-${version}/llvm-project-${version}.src.tar.xz"

export MSYS=winsymlinks:nativestrict # needed for windows
tar -xf "llvm-project-${version}.src.tar.xz"
mv "llvm-project-${version}.src" "$work_dir/src"

cmake -G Ninja \
      -S "$work_dir/src/llvm" -B "$work_dir/build" \
      -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_SHARED_LIBS=OFF \
      -DCMAKE_INSTALL_PREFIX="$install_dir" \
      -DLLVM_ENABLE_PROJECTS=clang \
      -DLLVM_INCLUDE_TESTS=OFF \
      -DLLVM_INCLUDE_BENCHMARKS=OFF
cmake --build "$work_dir/build"
cmake --install "$work_dir/build"

## TODO: figure out how to only build and install the libraries
##cmake --build "$work_dir/build" -t \
##  install-clang-headers \
##  install-clang-libraries \
##  install-clang-cmake-exports
