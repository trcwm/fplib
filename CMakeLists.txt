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

add_library(fplib src/fplib.cpp src/fplib.h)