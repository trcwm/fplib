#
# FPLIB is a library for arbitrary precision fixed-point
# calculations.
#
# Copyright (C) 2017 Niels A. Moseley
#

cmake_minimum_required (VERSION 3.0)
project (fplib)

# enable C++11 standard
set(CMAKE_CXX_STANDARD 11)

# enable intrinsics for clang compiler
if (CMAKE_CXX_COMPILER MATCHES ".*clang")
  set(CMAKE_CXX_FLAGS "-madx")
endif()

# enable exception unwinding semantics for
# Visual Studio
if (CMAKE_CXX_COMPILER MATCHES ".*Microsoft")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc") 
endif()

message("Using: ${CMAKE_CXX_COMPILER}")

add_library(fplib src/fplib.cpp src/fplib.h src/fpreference.cpp src/fpreference.h)
add_executable(fplib_tests tests/main.cpp tests/reftest.cpp)
target_link_libraries(fplib_tests fplib)
