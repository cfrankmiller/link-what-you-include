#!/bin/bash

find lib app -type f \( -name "*.cpp" -o -name "*.hpp" \) -exec clang-format -i {} +
