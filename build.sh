#!/usr/bin/sh

# Format the code
# find . -regex '.*\.\(cpp\|hpp\|cu\|cuh\|c\|h\)' -exec clang-format -style=file -i {} \;

# rm -rf build
#
# mkdir build

cd build

cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..

make -j$(nproc)
