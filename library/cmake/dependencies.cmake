
message(STATUS "Preparing dependencies for pixelmap...")

# Override policies
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

include(FetchContent)

set(ZLIB_VERSION 1.3.1)
set(DEFLATE_VERSION 1.21)
set(PNG_VERSION 1.6.43)
set(GLM_VERSION 1.0.1)
set(SPDLOG_VERSION 1.14.1)
# Need substantional changes to upgrade to 3.x
set(CATCH2_VERSION 2.13.9)

# Load packages
find_package(Threads REQUIRED)

# zlib
# NOTE: Due to the underlying repository, they limit themselves to specific sets of versions
FetchContent_Declare(
	zlib-cmake
	URL "https://github.com/jimmy-park/zlib-cmake/archive/${ZLIB_VERSION}.zip"
)

set(ZLIB_USE_STATIC_LIBS ON)
FetchContent_MakeAvailable(zlib-cmake)

# libdeflate
if (PIXELMAP_USE_LIBDEFLATE)
	FetchContent_Declare(
		DEFLATE
		GIT_REPOSITORY "https://github.com/ebiggers/libdeflate.git"
		GIT_TAG "v${DEFLATE_VERSION}"
	)

	set(LIBDEFLATE_BUILD_GZIP OFF)
	set(LIBDEFLATE_COMPRESSION_SUPPORT OFF)
	FetchContent_MakeAvailable(deflate)
	set(DEFLATE_LIBRARY libdeflate_static)
	set(DEFLATE_LIBRARIES ${DEFLATE_LIBRARY})
endif()

# png
FetchContent_Declare(
	PNG
	GIT_REPOSITORY "https://github.com/pnggroup/libpng.git"
	GIT_TAG "v${PNG_VERSION}"
)

set(PNG_TOOLS OFF)
set(PNG_SHARED OFF)
set(PNG_TESTS OFF)
set(SKIP_INSTALL_ALL ON)
FetchContent_MakeAvailable(png)
set(PNG_LIBRARY png_static)
set(PNG_LIBRARIES ${PNG_LIBRARY})
get_filename_component(PNG_INCLUDE_DIR "${png_SOURCE_DIR}" ABSOLUTE CACHE)
set(PNG_INCLUDE_DIRS "${PNG_INCLUDE_DIR}" CACHE PATH "PNG include dirs" FORCE)
target_include_directories(${PNG_LIBRARY} PUBLIC "${png_BINARY_DIR}")

# glm
FetchContent_Declare(
	GLM
	GIT_REPOSITORY "https://github.com/g-truc/glm.git"
	GIT_TAG "${GLM_VERSION}"
)

set(GLM_BUILD_TESTS OFF)
set(GLM_BUILD_INSTALL OFF)
FetchContent_MakeAvailable(glm)
set(GLM_LIBRARY glm)
set(GLM_LIBRARIES ${GLM_LIBRARY})
get_filename_component(GLM_INCLUDE_DIR "${glm_SOURCE_DIR}" ABSOLUTE CACHE)
set(GLM_INCLUDE_DIRS "${GLM_INCLUDE_DIR}" CACHE PATH "GLM include dirs" FORCE)

# spdlog
FetchContent_Declare(
	spdlog
	GIT_REPOSITORY "https://github.com/gabime/spdlog.git"
	GIT_TAG "v${SPDLOG_VERSION}"
	OVERRIDE_FIND_PACKAGE
)

set(SPDLOG_BUILD_SHARED OFF)
set(SPDLOG_BUILD_PIC ON)
FetchContent_MakeAvailable(spdlog)
set(SPDLOG_LIBRARY spdlog)
set(SPDLOG_LIBRARIES ${SPDLOG_LIBRARY})
get_filename_component(SPDLOG_INCLUDE_DIR "${spdlog_SOURCE_DIR}/include" ABSOLUTE CACHE)
set(SPDLOG_INCLUDE_DIRS "${SPDLOG_INCLUDE_DIR}" CACHE PATH "spdlog include dirs" FORCE)

if (PIXELMAP_BUILD_TESTS)
	# Catch2
	FetchContent_Declare(
		Catch2
		GIT_REPOSITORY "https://github.com/catchorg/Catch2.git"
		GIT_TAG "v${CATCH2_VERSION}"
	)

	FetchContent_MakeAvailable(Catch2)
	set(CATCH2_LIBRARY Catch2)
	set(CATCH2_LIBRARIES ${CATCH2_LIBRARY})
endif()

