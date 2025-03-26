cmake_minimum_required(VERSION 3.24)

project(PixelMapLib VERSION 2.0.0 LANGUAGES CXX)

# Options
option(PIXELMAP_USE_LIBDEFLATE "Use libdeflate optimization" ON)
option(PIXELMAP_ENABLE_AFFINITY "Enable thread affinity" OFF)
option(PIXELMAP_PROFILE "Profile performance on separate sections " OFF)

# Set the desired C++ version
set(CMAKE_CXX_STANDARD 17)

# Add cmake directory to module path
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Options
option(BUILD_SHARED_LIBS "Build everything as shared libs" OFF)
option(CMAKE_BUILD_TYPE "Set build type: Release, Debug" Release)

# Make Windows happy
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include(dependencies)
include(embedfile)

# Subdirectories
add_subdirectory("src")
if (PIXELMAP_BUILD_TESTS)
	add_subdirectory("tests")
	include(test)
endif()

# Configuration files
configure_file(pixelmap-config-version.cmake ${CMAKE_CURRENT_BINARY_DIR}/pixelmap-config-version.cmake @ONLY)
