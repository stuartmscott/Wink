#!/bin/sh

clang-format -style=Google -i src/*.cpp src/*/*.cpp include/Wink/*.h include/WinkServer/*.h test/src/*.cpp test/include/WinkTest/*.h samples/*/*.cpp samples/*/include/*.h
