#!/bin/sh

xcrun run-clang-tidy -p build
#cpplint src/*.cpp src/*/*.cpp include/Wink/*.h include/WinkServer/*.h test/src/*.cpp test/include/WinkTest/*.h samples/*/*.cpp
