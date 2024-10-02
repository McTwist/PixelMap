cmake_minimum_required(VERSION 3.6)

project(PixelMap VERSION 2.0.0 LANGUAGES CXX)

# Options
option(PIXELMAP_DEBUG_MODE "Able to debug the program" OFF)
option(PIXELMAP_BUILD_CLI "Build the CLI executable" ON)
option(PIXELMAP_BUILD_GUI "Build the GUI executable" ON)

# Options
option(BUILD_SHARED_LIBS "Build everything as shared libs" OFF)
option(CMAKE_BUILD_TYPE "Set build type: Release, Debug" Release)

# Make Windows happy
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Subdirectories
add_subdirectory("library")
if (PIXELMAP_BUILD_CLI)
	add_subdirectory("cli")
endif()
if (PIXELMAP_BUILD_GUI)
	add_subdirectory("gui")
endif()
