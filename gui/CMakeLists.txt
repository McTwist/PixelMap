cmake_minimum_required(VERSION 3.24)

project(PixelMapIm LANGUAGES CXX)

# Options
option(PIXELMAPIM_DEBUG_MODE "Able to debug the program" OFF)
option(PIXELMAPIM_INSTALL "Install the program" ${PIXELMAP_INSTALL})
option(USE_OPENGL "Use OpenGL for render" OFF)

# Set the desired C++ version
set(CMAKE_CXX_STANDARD 17)

# Add cmake directory to module path
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../library/cmake)

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

if (PIXELMAPIM_INSTALL)
	include(installgui)
endif()
