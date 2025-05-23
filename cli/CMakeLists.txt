cmake_minimum_required(VERSION 3.6)

project(PixelMapCli LANGUAGES CXX)

# Options
option(PIXELMAPCLI_DEBUG_MODE "Able to debug the program" OFF)
option(PIXELMAPCLI_INSTALL "Install the program" ${PIXELMAP_INSTALL})
option(PIXELMAPCLI_USE_SPDMON "Make use of spdmon" ON)
option(PIXELMAP_PROFILE "Enable the memory profiler" OFF)

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

# Subdirectories
add_subdirectory("src")

if (PIXELMAPCLI_INSTALL)
	include(installcli)
endif()
