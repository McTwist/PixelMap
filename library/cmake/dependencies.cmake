
message(STATUS "Preparing dependencies for pixelmap...")

# Override policies
# "option() honors normal variables.""
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
# "Prefer files from CMake module directory when invluding from there."
set(CMAKE_POLICY_DEFAULT_CMP0017 NEW)
# "Automatically link Qt executables to qtmain target on Windows."
# Note: Windows complains about this, but we do not use Qt.
set(CMAKE_POLICY_DEFAULT_CMP0020 NEW)

include(FetchContent)
include(FetchContent_MakeAvailableExcludeFromAll)

set(ZLIB_VERSION 1.3.1)
set(DEFLATE_VERSION 1.21)
set(LZ4_VERSION 1.10.0)
set(PNG_VERSION 1.6.43)
set(GLM_VERSION 1.0.1)
set(FMT_VERSION 11.0.2)
set(SPDLOG_VERSION 1.14.1)
# Need substantional changes to upgrade to 3.x
set(CATCH2_VERSION 2.13.9)

# Load packages
find_package(Threads REQUIRED)

# zlib
FetchContent_Declare(
	ZLIB
	GIT_REPOSITORY "https://github.com/madler/zlib.git"
	GIT_TAG "v${ZLIB_VERSION}"
	EXCLUDE_FROM_ALL
)

set(ZLIB_USE_SHARED_LIBS ${BUILD_SHARED_LIBS})
set(ZLIB_USE_STATIC_LIBS ${BUILD_STATIC_LIBS})
set(ZLIB_BUILD_EXAMPLES OFF)
FetchContent_MakeAvailableExcludeFromAll(ZLIB)
if (${BUILD_SHARED_LIBS})
	add_library(ZLIB::ZLIB ALIAS zlib)
	set(ZLIB_LIBRARY zlib)
else()
	add_library(ZLIB::ZLIB ALIAS zlibstatic)
	set(ZLIB_LIBRARY zlibstatic)
endif()
set(ZLIB_LIBRARIES ${ZLIB_LIBRARY})
get_filename_component(ZLIB_INCLUDE_DIR "${zlib_SOURCE_DIR};${zlib_BINARY_DIR}" ABSOLUTE)
set(ZLIB_INCLUDE_DIRS "${ZLIB_INCLUDE_DIR}")
target_include_directories(${ZLIB_LIBRARY} PUBLIC "${zlib_BINARY_DIR}")
set(ZLIB_LICENSE_FILE "${zlib_SOURCE_DIR}/LICENSE" PARENT_SCOPE)

# libdeflate
if (PIXELMAP_USE_LIBDEFLATE)
	FetchContent_Declare(
		DEFLATE
		GIT_REPOSITORY "https://github.com/ebiggers/libdeflate.git"
		GIT_TAG "v${DEFLATE_VERSION}"
		EXCLUDE_FROM_ALL
	)

	set(LIBDEFLATE_BUILD_GZIP OFF)
	set(LIBDEFLATE_COMPRESSION_SUPPORT OFF) # We do not need this
	set(LIBDEFLATE_BUILD_SHARED_LIB ${BUILD_SHARED_LIBS})
	set(LIBDEFLATE_BUILD_STATIC_LIB ${BUILD_STATIC_LIBS})
	set(LIBDEFLATE_USE_SHARED_LIB ${BUILD_SHARED_LIBS})
	FetchContent_MakeAvailable(DEFLATE)
	if (${BUILD_SHARED_LIBS})
		set(DEFLATE_LIBRARY libdeflate_shared)
	else()
		set(DEFLATE_LIBRARY libdeflate_static)
	endif()
	set(DEFLATE_LIBRARIES ${DEFLATE_LIBRARY})
	set(DEFLATE_LICENSE_FILE "${deflate_SOURCE_DIR}/COPYING" PARENT_SCOPE)
endif()

# lz4
FetchContent_Declare(
	LZ4
	GIT_REPOSITORY "https://github.com/lz4/lz4.git"
	GIT_TAG "v${LZ4_VERSION}"
	EXCLUDE_FROM_ALL
)

set(LZ4_BUILD_CLI OFF)
FetchContent_MakeAvailable(LZ4)
if (${BUILD_SHARED_LIBS})
	set(LZ4_LIBRARY lz4)
else()
	set(LZ4_LIBRARY lz4_static)
endif()
set(LZ4_LIBRARIES ${LZ4_LIBRARY})
get_filename_component(LZ4_INCLUDE_DIR "${lz4_SOURCE_DIR}" ABSOLUTE)
set(LZ4_INCLUDE_DIRS "${LZ4_INCLUDE_DIR}/lib")
add_subdirectory("${lz4_SOURCE_DIR}/build/cmake" "${lz4_BINARY_DIR}")
set(LZ4_LICENSE_FILE "${lz4_SOURCE_DIR}/LICENSE" PARENT_SCOPE)

# png
FetchContent_Declare(
	PNG
	GIT_REPOSITORY "https://github.com/pnggroup/libpng.git"
	GIT_TAG "v${PNG_VERSION}"
	EXCLUDE_FROM_ALL
)

set(PNG_TOOLS OFF)
set(PNG_SHARED ${BUILD_SHARED_LIBS})
set(PNG_TESTS OFF)
set(SKIP_INSTALL_ALL ON)
FetchContent_MakeAvailable(PNG)
if (${BUILD_SHARED_LIBS})
	set(PNG_LIBRARY png_shared)
else()
	set(PNG_LIBRARY png_static)
endif()
set(PNG_LIBRARIES ${PNG_LIBRARY})
get_filename_component(PNG_INCLUDE_DIR "${png_SOURCE_DIR}" ABSOLUTE)
set(PNG_INCLUDE_DIRS "${PNG_INCLUDE_DIR}")
target_include_directories(${PNG_LIBRARY} PUBLIC "${png_BINARY_DIR}")
set(PNG_LICENSE_FILE "${png_SOURCE_DIR}/LICENSE" PARENT_SCOPE)

# glm
FetchContent_Declare(
	GLM
	GIT_REPOSITORY "https://github.com/g-truc/glm.git"
	GIT_TAG "${GLM_VERSION}"
	EXCLUDE_FROM_ALL
)

set(GLM_BUILD_TESTS OFF)
set(GLM_BUILD_INSTALL OFF)
FetchContent_MakeAvailable(GLM)
set(GLM_LIBRARY glm)
set(GLM_LIBRARIES ${GLM_LIBRARY})
get_filename_component(GLM_INCLUDE_DIR "${glm_SOURCE_DIR}" ABSOLUTE)
set(GLM_INCLUDE_DIRS "${GLM_INCLUDE_DIR}")
set(GLM_LICENSE_FILE "${glm_SOURCE_DIR}/copying.txt" PARENT_SCOPE)

# fmt
FetchContent_Declare(
	fmt
	GIT_REPOSITORY "https://github.com/fmtlib/fmt.git"
	GIT_TAG "${FMT_VERSION}"
	EXCLUDE_FROM_ALL
)

set(FMT_TEST OFF)
set(FMT_DOC OFF)
set(FMT_INSTALL OFF)
FetchContent_MakeAvailable(fmt)
set(FMT_LIBRARY fmt)
set(FMT_LIBRARIES ${FMT_LIBRARY})
get_filename_component(FMT_INCLUDE_DIR "${fmt_SOURCE_DIR}" ABSOLUTE)
set(FMT_INCLUDE_DIRS "${FMT_INCLUDE_DIR}/include")
set(FMT_LICENSE_FILE "${fmt_SOURCE_DIR}/LICENSE" PARENT_SCOPE)

# spdlog
FetchContent_Declare(
	spdlog
	GIT_REPOSITORY "https://github.com/gabime/spdlog.git"
	GIT_TAG "v${SPDLOG_VERSION}"
	EXCLUDE_FROM_ALL
	OVERRIDE_FIND_PACKAGE
)

set(SPDLOG_BUILD_SHARED ${BUILD_SHARED_LIBS})
set(SPDLOG_BUILD_PIC ON)
set(SPDLOG_FMT_EXTERNAL ON)
FetchContent_MakeAvailable(spdlog)
set(SPDLOG_LIBRARY spdlog)
set(SPDLOG_LIBRARIES ${SPDLOG_LIBRARY})
get_filename_component(SPDLOG_INCLUDE_DIR "${spdlog_SOURCE_DIR}/include" ABSOLUTE)
set(SPDLOG_INCLUDE_DIRS "${SPDLOG_INCLUDE_DIR}")
set(SPDLOG_LICENSE_FILE "${spdlog_SOURCE_DIR}/LICENSE" PARENT_SCOPE)

if (PIXELMAP_BUILD_TESTS)
	# Catch2
	FetchContent_Declare(
		Catch2
		GIT_REPOSITORY "https://github.com/catchorg/Catch2.git"
		GIT_TAG "v${CATCH2_VERSION}"
		EXCLUDE_FROM_ALL
	)

	FetchContent_MakeAvailable(Catch2)
	set(CATCH2_LIBRARY Catch2)
	set(CATCH2_LIBRARIES ${CATCH2_LIBRARY})
	set(CATCH2_LICENSE_FILE "${catch2_SOURCE_DIR}/LICENSE.txt" PARENT_SCOPE)
endif()

