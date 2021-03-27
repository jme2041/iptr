# CMakeLists.txt for project iptr #############################################
#
# Copyright (c) 2021, Jeffrey M. Engelmann
#
# iptr is released under the MIT license.
# For details, see LICENSE.txt
#

cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

project(iptr VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

# Require out-of-source builds
if( EXISTS "${PROJECT_BINARY_DIR}/CMakeLists.txt" OR
    EXISTS "${PROJECT_BINARY_DIR}/iptr.h" OR
    EXISTS "${PROJECT_BINARY_DIR}/test_iptr.cpp")
    message(FATAL_ERROR "Cannot build in source directory")
endif()

# Set the build type (default is Release)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release"
            CACHE STRING "Choose the type of build" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
            "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Build the test program
add_executable(test_iptr "test/test_iptr.cpp")
target_include_directories(test_iptr PRIVATE "include")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(test_iptr PRIVATE IPTR_SHOW_TRACE)
endif()

###############################################################################
