
# Set paths
set(DOWNLOAD_PATH ${PROJECT_BINARY_DIR}/downloads)
set(MODULES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../modules)
file(MAKE_DIRECTORY ${DOWNLOAD_PATH})
file(MAKE_DIRECTORY ${MODULES_PATH})

message(STATUS "Preparing dependencies for pixelmap...")

set(ZLIB_VERSION 1.3.1)
set(DEFLATE_VERSION 1.21)
set(PNG_VERSION 1.6.43)
set(GLM_VERSION 1.0.1)
set(SPDLOG_VERSION 1.14.1)
# Need substantional changes to upgrade to 3.x
set(CATCH2_VERSION 2.13.9)

# Threads
set(THREADS_PREFER_PTHREAD_FLAG ON)

# Load packages
find_package(Threads REQUIRED)
find_package(ZLIB)
find_package(DEFLATE)
find_package(PNG)
find_package(GLM)
#find_package(SPDLOG) # Disabled for now, as it does not go along well with fmt

if (NOT ZLIB_FOUND)
	# Note: https://stackoverflow.com/a/23383854
	set(ZLIB_NAME "zlib-${ZLIB_VERSION}")
	# Download
	set(ZLIB_URL "http://zlib.net/${ZLIB_NAME}.tar.gz")
	set(ZLIB_DOWNLOAD_PATH ${DOWNLOAD_PATH}/${ZLIB_NAME}.tar.gz)
	set(ZLIB_EXTRACTED_FILE ${MODULES_PATH})

	if (NOT EXISTS "${ZLIB_DOWNLOAD_PATH}")
		message(STATUS "Downloading ${ZLIB_NAME}")
		file(DOWNLOAD "${ZLIB_URL}" "${ZLIB_DOWNLOAD_PATH}")
	endif()

	# Extract
	if (NOT EXISTS "${ZLIB_EXTRACTED_FILE}/${ZLIB_NAME}")
		execute_process(
			COMMAND ${CMAKE_COMMAND} -E tar xzf ${ZLIB_DOWNLOAD_PATH}
			WORKING_DIRECTORY ${ZLIB_EXTRACTED_FILE})
	endif()

	set(ZLIB_USE_STATIC_LIBS "ON")
	set(ZLIB_DIR "${MODULES_PATH}/${ZLIB_NAME}")
	add_subdirectory(${ZLIB_DIR} "${PROJECT_BINARY_DIR}/modules/${ZLIB_NAME}")

	# Output variables
	set(ZLIB_LIBRARY zlib)
	set(ZLIB_LIBRARIES ${ZLIB_LIBRARY})
	get_filename_component(ZLIB_INCLUDE_DIR "${ZLIB_DIR}" ABSOLUTE CACHE)
	set(ZLIB_INCLUDE_DIRS ${ZLIB_INCLUDE_DIR} "${PROJECT_BINARY_DIR}/modules/${ZLIB_NAME}" CACHE PATH "zlib include dirs" FORCE)
	set(ZLIB_DEPEND true)
endif()

if (NOT DEFLATE_FOUND)
	set(DEFLATE_NAME "libdeflate-${DEFLATE_VERSION}")
	# Download
	set(DEFLATE_URL "https://github.com/ebiggers/libdeflate/archive/refs/tags/v${DEFLATE_VERSION}.tar.gz")
	set(DEFLATE_DOWNLOAD_PATH ${DOWNLOAD_PATH}/${DEFLATE_NAME}.tar.gz)
	set(DEFLATE_EXTRACTED_FILE ${MODULES_PATH})

	if (NOT EXISTS "${DEFLATE_DOWNLOAD_PATH}")
		message(STATUS "Downloading ${DEFLATE_NAME}")
		file(DOWNLOAD "${DEFLATE_URL}" "${DEFLATE_DOWNLOAD_PATH}")
	endif()

	# Extract
	if (NOT EXISTS "${DEFLATE_EXTRACTED_FILE}/${DEFLATE_NAME}")
		execute_process(
			COMMAND ${CMAKE_COMMAND} -E tar xzf ${DEFLATE_DOWNLOAD_PATH}
			WORKING_DIRECTORY ${DEFLATE_EXTRACTED_FILE})
	endif()

	set(DEFLATE_PREFER_STATIC_LIB ON)
	set(DEFLATE_DIR "${MODULES_PATH}/${DEFLATE_NAME}")
	add_subdirectory(${DEFLATE_DIR} "${PROJECT_BINARY_DIR}/modules/${DEFLATE_NAME}")

	set(DEFLATE_LIBRARY deflate)
	set(DEFLATE_LIBRARIES ${DEFLATE_LIBRARY})
	get_filename_component(DEFLATE_INCLUDE_DIR "${DEFLATE_DIR}" ABSOLUTE CACHE)
	set(DEFLATE_INCLUDE_DIRS ${DEFLATE_INCLUDE_DIR} "${PROJECT_BINARY_DIR}/modules/${DEFLATE_NAME}/src" CACHE PATH "deflate include dirs" FORCE)
	set(DEFLATE_DEPEND true)
endif()

if (NOT PNG_FOUND)
	set(PNG_NAME "libpng-${PNG_VERSION}")
	# Download
	set(PNG_URL "http://prdownloads.sourceforge.net/libpng/${PNG_NAME}.tar.gz?download")
	set(PNG_DOWNLOAD_PATH ${DOWNLOAD_PATH}/${PNG_NAME}.tar.gz)
	set(PNG_EXTRACTED_FILE ${MODULES_PATH})

	if (NOT EXISTS "${PNG_DOWNLOAD_PATH}")
		message(STATUS "Downloading ${PNG_NAME}")
		file(DOWNLOAD "${PNG_URL}" "${PNG_DOWNLOAD_PATH}")
	endif()

	# Extract
	if (NOT EXISTS "${PNG_EXTRACTED_FILE}/${PNG_NAME}")
		execute_process(
			COMMAND ${CMAKE_COMMAND} -E tar xzf ${PNG_DOWNLOAD_PATH}
			WORKING_DIRECTORY ${PNG_EXTRACTED_FILE})
	endif()

	set(PNG_DIR "${MODULES_PATH}/${PNG_NAME}")

	# Turn off all tests
	set(PNG_EXECUTABLES OFF)
	set(PNG_SHARED OFF)
	set(PNG_TESTS OFF)
	set(SKIP_INSTALL_ALL ON)
	set(PNG_BUILD_ZLIB ON)
	add_subdirectory(${PNG_DIR} "${PROJECT_BINARY_DIR}/modules/${PNG_NAME}")

	# Output variables
	set(PNG_LIBRARY png)
	set(PNG_LIBRARIES ${PNG_LIBRARY})
	get_filename_component(PNG_INCLUDE_DIR "${PNG_DIR}" ABSOLUTE CACHE)
	set(PNG_INCLUDE_DIRS "${PNG_INCLUDE_DIR}" "${PROJECT_BINARY_DIR}/modules/${PNG_NAME}" CACHE PATH "png include dirs" FORCE)
	set(PNG_PNG_INCLUDE_DIR "${PNG_INCLUDE_DIRS}" CACHE PATH "png include dir" FORCE)
	set(PNG_DEPEND true)
	#set_property(TARGET ${PNG_LIBRARY} PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

# glm dependency
if (NOT GLM_FOUND)
	set(GLM_NAME "glm-${GLM_VERSION}")
	# Download
	set(GLM_URL "https://github.com/g-truc/glm/archive/${GLM_VERSION}.tar.gz")
	set(GLM_DOWNLOAD_PATH ${DOWNLOAD_PATH}/${GLM_NAME}.tar.gz)
	set(GLM_EXTRACTED_FILE ${MODULES_PATH})

	if (NOT EXISTS "${GLM_DOWNLOAD_PATH}")
		message(STATUS "Downloading ${GLM_NAME}")
		file(DOWNLOAD "${GLM_URL}" "${GLM_DOWNLOAD_PATH}")
	endif()

	# Extract
	if (NOT EXISTS "${GLM_EXTRACTED_FILE}/${GLM_NAME}")
		execute_process(
			COMMAND ${CMAKE_COMMAND} -E tar xzf ${GLM_DOWNLOAD_PATH}
			WORKING_DIRECTORY ${GLM_EXTRACTED_FILE})
	endif()

	set(GLM_DIR "${MODULES_PATH}/${GLM_NAME}")

	add_subdirectory(${GLM_DIR} "${PROJECT_BINARY_DIR}/modules/${GLM_NAME}")

	# Output variables
	set(GLM_LIBRARY glm)
	set(GLM_LIBRARIES ${GLM_LIBRARY})
	get_filename_component(GLM_INCLUDE_DIR "${GLM_DIR}" ABSOLUTE CACHE)
	set(GLM_INCLUDE_DIRS ${GLM_INCLUDE_DIR} CACHE PATH "glm include dirs" FORCE)
	set(GLM_DEPEND true)
endif()

if (NOT SPDLOG_FOUND)
	set(SPDLOG_NAME "spdlog-${SPDLOG_VERSION}")
	# Download
	set(SPDLOG_URL "https://github.com/gabime/spdlog/archive/v${SPDLOG_VERSION}.tar.gz")
	set(SPDLOG_DOWNLOAD_PATH ${DOWNLOAD_PATH}/${SPDLOG_NAME}.tar.gz)
	set(SPDLOG_EXTRACTED_FILE ${MODULES_PATH})

	if (NOT EXISTS "${SPDLOG_DOWNLOAD_PATH}")
		message(STATUS "Downloading ${SPDLOG_NAME}")
		file(DOWNLOAD "${SPDLOG_URL}" "${SPDLOG_DOWNLOAD_PATH}")
	endif()

	# Extract
	if (NOT EXISTS "${SPDLOG_EXTRACTED_FILE}/${SPDLOG_NAME}")
		execute_process(
			COMMAND ${CMAKE_COMMAND} -E tar xzf ${SPDLOG_DOWNLOAD_PATH}
			WORKING_DIRECTORY ${SPDLOG_EXTRACTED_FILE})
	endif()

	set(SPDLOG_BUILD_SHARED OFF)
	set(SPDLOG_BUILD_PIC ON)

	set(SPDLOG_DIR "${MODULES_PATH}/${SPDLOG_NAME}")
	add_subdirectory(${SPDLOG_DIR} "${PROJECT_BINARY_DIR}/modules/${SPDLOG_NAME}")

	# Output variables
	set(SPDLOG_LIBRARY spdlog)
	set(SPDLOG_LIBRARIES ${SPDLOG_LIBRARY})
	get_filename_component(SPDLOG_INCLUDE_DIR "${SPDLOG_DIR}/include" ABSOLUTE CACHE)
	set(SPDLOG_INCLUDE_DIRS ${SPDLOG_INCLUDE_DIR} CACHE PATH "spdlog include dirs" FORCE)
	set(SPDLOG_DEPEND true)
endif()

if (NOT CATCH2_FOUND AND PIXELMAP_BUILD_TESTS)
	# Note: v3 exist, spend time converting
	set(CATCH2_NAME "Catch2-${CATCH2_VERSION}")
	# Download
	set(CATCH2_URL "https://github.com/catchorg/Catch2/archive/v${CATCH2_VERSION}.tar.gz")
	set(CATCH2_DOWNLOAD_PATH ${DOWNLOAD_PATH}/${CATCH2_NAME}.tar.gz)
	set(CATCH2_EXTRACTED_FILE ${MODULES_PATH})

	if (NOT EXISTS "${CATCH2_DOWNLOAD_PATH}")
		message(STATUS "Downloading ${CATCH2_NAME}")
		file(DOWNLOAD "${CATCH2_URL}" "${CATCH2_DOWNLOAD_PATH}")
	endif()

	# Extract
	if (NOT EXISTS "${CATCH2_EXTRACTED_FILE}/${CATCH2_NAME}")
		execute_process(
			COMMAND ${CMAKE_COMMAND} -E tar xzf ${CATCH2_DOWNLOAD_PATH}
			WORKING_DIRECTORY ${CATCH2_EXTRACTED_FILE})
	endif()

	#add_definitions( -DCATCH_CONFIG_ENABLE_BENCHMARKING )

	set(CATCH2_DIR "${MODULES_PATH}/${CATCH2_NAME}")
	add_subdirectory(${CATCH2_DIR} "${PROJECT_BINARY_DIR}/modules/${CATCH2_NAME}")

	# Output variables
	set(CATCH2_LIBRARY Catch2)
	set(CATCH2_LIBRARIES ${CATCH2_LIBRARY})
	get_filename_component(CATCH2_INCLUDE_DIR "${CATCH2_DIR}/include" ABSOLUTE CACHE)
	set(CATCH2_INCLUDE_DIRS ${CATCH2_INCLUDE_DIR} CACHE PATH "catch2 include dirs" FORCE)
endif()

