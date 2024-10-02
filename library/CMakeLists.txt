cmake_minimum_required(VERSION 3.1)

project(PixelMapLib VERSION 2.0.0 LANGUAGES CXX)

# Options
option(PIXELMAP_DEBUG_MODE "Set to ON to debug the library" OFF)
option(PIXELMAP_BUILD_TESTS "Build the tests exetutable" ON)

# Set the desired C++ version
set(CMAKE_CXX_STANDARD 17)

# Add cmake directory to module path
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Warning levels
if(MSVC)
	# Force to always compile with W4
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
	# Update if necessary
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic -Wextra")
endif()

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
endif()

# Configuration files
configure_file(pixelmap-config-version.cmake ${CMAKE_CURRENT_BINARY_DIR}/pixelmap-config-version.cmake @ONLY)
