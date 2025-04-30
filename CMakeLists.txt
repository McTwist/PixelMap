cmake_minimum_required(VERSION 3.6)

project(PixelMap VERSION 1.21.5 LANGUAGES CXX)

# Options
option(PIXELMAP_BUILD_CLI "Build the CLI executable" ON)
option(PIXELMAP_BUILD_GUI "Build the GUI executable" ON)
option(PIXELMAP_BUILD_WEB "Build the WebView dependency" ON)
option(PIXELMAP_BUILD_TESTS "Build the tests executable" ON)
option(PIXELMAP_INSTALL "Install the program" OFF)

# Add cmake directory to module path
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

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

# Prepare cpack
set(CPACK_PACKAGE_CONTACT "mctwist@aposoc.net")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Minecraft mapper")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_STRIP_FILES YES)

# Fetch git revision
execute_process(COMMAND
	"${GIT_EXECUTABLE}" describe --match=NeVeRmAtCh --always --abbrev=10
	WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
	OUTPUT_VARIABLE GIT_REVISION
	ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

configure_file("version.hpp.in" "${CMAKE_CURRENT_BINARY_DIR}/include/version.hpp" @ONLY)

# Root for enabling tests
if (PIXELMAP_BUILD_TESTS)
	enable_testing()
endif()

# Subdirectories
if (PIXELMAP_BUILD_WEB)
	add_subdirectory("webview")
endif()
add_subdirectory("library")
if (PIXELMAP_BUILD_CLI)
	add_subdirectory("cli")
	target_include_directories(pixelmapcli PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/include")
endif()
if (PIXELMAP_BUILD_GUI)
	add_subdirectory("gui")
	target_include_directories(pixelmapim PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/include")
endif()
