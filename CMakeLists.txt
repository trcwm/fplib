#
# FPLIB is a library for arbitrary precision fixed-point
# calculations.
#
# Copyright (C) 2017 Niels A. Moseley
#

cmake_minimum_required (VERSION 3.0)
project (fplib)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  target_compile_options(fplib PRIVATE -madx)
endif()

add_library(fplib src/fplib.cpp src/fplib.h)