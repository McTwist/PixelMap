cmake_minimum_required(VERSION 3.6)

project(PixelMap VERSION 1.21.3 LANGUAGES CXX)

# Options
option(PIXELMAP_DEBUG_MODE "Able to debug the program" OFF)
option(PIXELMAP_BUILD_CLI "Build the CLI executable" ON)
option(PIXELMAP_BUILD_GUI "Build the GUI executable" ON)

# Options
option(BUILD_SHARED_LIBS "Build everything as shared libs" OFF)
option(CMAKE_BUILD_TYPE "Set build type: Release, Debug" Release)

if (${BUILD_SHARED_LIBS})
	set(BUILD_STATIC_LIBS OFF)
else()
	set(BUILD_STATIC_LIBS ON)
endif()

# Make Windows happy
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

execute_process(COMMAND
	"${GIT_EXECUTABLE}" describe --match=NeVeRmAtCh --always --abbrev=10
	WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
	OUTPUT_VARIABLE GIT_REVISION
	ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

configure_file("version.hpp.in" "${CMAKE_CURRENT_BINARY_DIR}/include/version.hpp" @ONLY)

# Subdirectories
add_subdirectory("library")
if (PIXELMAP_BUILD_CLI)
	add_subdirectory("cli")
	target_include_directories(pixelmapcli PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/include")
endif()
if (PIXELMAP_BUILD_GUI)
	add_subdirectory("gui")
	target_include_directories(pixelmapim PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/include")
endif()
