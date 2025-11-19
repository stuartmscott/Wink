cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

string(APPEND CMAKE_CXX_FLAGS " -std=c++20")
string(APPEND CMAKE_CXX_FLAGS " -Wall")
string(APPEND CMAKE_CXX_FLAGS " -Werror")
string(APPEND CMAKE_CXX_FLAGS " -pedantic")

set(PROJECT_NAME Wink)

project(${PROJECT_NAME} VERSION 0.0.1)

set(CMAKE_STATIC_LIBRARY_PREFIX "")

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_subdirectory("src")

add_subdirectory("samples")

# Set the timestamps of extracted contents to the time of extraction
if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

enable_testing()
add_subdirectory("test/src")
