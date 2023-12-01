
# Set paths
set(DOWNLOAD_PATH ${PROJECT_BINARY_DIR}/downloads)
set(MODULES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../modules)
file(MAKE_DIRECTORY ${DOWNLOAD_PATH})
file(MAKE_DIRECTORY ${MODULES_PATH})

message(STATUS "Preparing dependencies for pixelmap...")

set(ZLIB_VERSION 1.3)
set(LIBPNG_VERSION 1.6.39)
set(GLM_VERSION 0.9.9.8)
set(SPDLOG_VERSION 1.12.0)
set(CATCH2_VERSION 2.13.9)

# Threads
set(THREADS_PREFER_PTHREAD_FLAG ON)

# Load packages
find_package(Threads REQUIRED)
find_package(ZLIB)
find_package(LIBPNG)
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
		message("Downloading ${ZLIB_NAME}")
		file(DOWNLOAD "${ZLIB_URL}" "${ZLIB_DOWNLOAD_PATH}")
	endif()

	# Extract
	if (NOT EXISTS "${ZLIB_EXTRACTED_FILE}/${ZLIB_NAME}")
		execute_process(
			COMMAND ${CMAKE_COMMAND} -E tar xzf ${ZLIB_DOWNLOAD_PATH}
			WORKING_DIRECTORY ${ZLIB_EXTRACTED_FILE})
	endif()

	set(ZLIB_USE_STATIC_LIBS "ON")
	set(ZLIB_DIR ${MODULES_PATH}/${ZLIB_NAME})
	add_subdirectory(${ZLIB_DIR} ${PROJECT_BINARY_DIR}/modules/${ZLIB_NAME})

	# Output variables
	set(ZLIB_LIBRARY zlib)
	get_filename_component(ZLIB_INCLUDE_DIR ${ZLIB_DIR} ABSOLUTE)
	set(ZLIB_LIBRARIES ${ZLIB_LIBRARY})
	set(ZLIB_INCLUDE_DIRS ${ZLIB_INCLUDE_DIR} ${PROJECT_BINARY_DIR}/modules/${ZLIB_NAME})
	set(ZLIB_DEPEND true)
endif()

if (NOT LIBPNG_FOUND)
	set(LIBPNG_NAME "libpng-${LIBPNG_VERSION}")
	# Download
	set(LIBPNG_URL "http://prdownloads.sourceforge.net/libpng/${LIBPNG_NAME}.tar.gz?download")
	set(LIBPNG_DOWNLOAD_PATH ${DOWNLOAD_PATH}/${LIBPNG_NAME}.tar.gz)
	set(LIBPNG_EXTRACTED_FILE ${MODULES_PATH})

	if (NOT EXISTS "${LIBPNG_DOWNLOAD_PATH}")
		message("Downloading ${LIBPNG_NAME}")
		file(DOWNLOAD "${LIBPNG_URL}" "${LIBPNG_DOWNLOAD_PATH}")
	endif()

	# Extract
	if (NOT EXISTS "${LIBPNG_EXTRACTED_FILE}/${LIBPNG_NAME}")
		execute_process(
			COMMAND ${CMAKE_COMMAND} -E tar xzf ${LIBPNG_DOWNLOAD_PATH}
			WORKING_DIRECTORY ${LIBPNG_EXTRACTED_FILE})
	endif()

	set(LIBPNG_DIR ${MODULES_PATH}/${LIBPNG_NAME})

	# Turn off all tests
	set(PNG_TESTS OFF)
	set(SKIP_INSTALL_ALL ON)
	option(PNG_BUILD_ZLIB "Custom zlib location, else find_package is used" ON)
	add_definitions(-fPIC)
	add_subdirectory(${LIBPNG_DIR} ${PROJECT_BINARY_DIR}/modules/${LIBPNG_NAME})

	# Output variables
	set(LIBPNG_LIBRARY png)
	get_filename_component(LIBPNG_INCLUDE_DIR ${LIBPNG_DIR} ABSOLUTE)
	set(LIBPNG_LIBRARIES ${LIBPNG_LIBRARY})
	set(LIBPNG_INCLUDE_DIRS ${LIBPNG_INCLUDE_DIR} ${PROJECT_BINARY_DIR}/modules/${LIBPNG_NAME})
	set(LIBPNG_DEPEND true)
endif()

# glm dependency
if (NOT GLM_FOUND)
	set(GLM_NAME "glm-${GLM_VERSION}")
	# Download
	set(GLM_URL "https://github.com/g-truc/glm/archive/${GLM_VERSION}.tar.gz")
	set(GLM_DOWNLOAD_PATH ${DOWNLOAD_PATH}/${GLM_NAME}.tar.gz)
	set(GLM_EXTRACTED_FILE ${MODULES_PATH})

	if (NOT EXISTS "${GLM_DOWNLOAD_PATH}")
		message("Downloading ${GLM_NAME}")
		file(DOWNLOAD "${GLM_URL}" "${GLM_DOWNLOAD_PATH}")
	endif()

	# Extract
	if (NOT EXISTS "${GLM_EXTRACTED_FILE}/${GLM_NAME}")
		execute_process(
			COMMAND ${CMAKE_COMMAND} -E tar xzf ${GLM_DOWNLOAD_PATH}
			WORKING_DIRECTORY ${GLM_EXTRACTED_FILE})
	endif()

	set(GLM_DIR ${MODULES_PATH}/${GLM_NAME})

	add_subdirectory(${GLM_DIR} ${PROJECT_BINARY_DIR}/modules/${GLM_NAME})

	# Output variables
	set(GLM_LIBRARY glm)
	get_filename_component(GLM_INCLUDE_DIR ${GLM_DIR} ABSOLUTE)
	set(GLM_LIBRARIES ${GLM_LIBRARY})
	set(GLM_INCLUDE_DIRS ${GLM_INCLUDE_DIR})
	set(GLM_DEPEND true)
endif()

if (NOT SPDLOG_FOUND)
	set(SPDLOG_NAME "spdlog-${SPDLOG_VERSION}")
	# Download
	set(SPDLOG_URL "https://github.com/gabime/spdlog/archive/v${SPDLOG_VERSION}.tar.gz")
	set(SPDLOG_DOWNLOAD_PATH ${DOWNLOAD_PATH}/${SPDLOG_NAME}.tar.gz)
	set(SPDLOG_EXTRACTED_FILE ${MODULES_PATH})

	if (NOT EXISTS "${SPDLOG_DOWNLOAD_PATH}")
		message("Downloading ${SPDLOG_NAME}")
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

	set(SPDLOG_DIR ${MODULES_PATH}/${SPDLOG_NAME})
	add_subdirectory(${SPDLOG_DIR} ${PROJECT_BINARY_DIR}/modules/${SPDLOG_NAME})

	# Output variables
	set(SPDLOG_LIBRARY spdlog)
	get_filename_component(SPDLOG_INCLUDE_DIR ${SPDLOG_DIR}/include ABSOLUTE)
	set(SPDLOG_LIBRARIES ${SPDLOG_LIBRARY})
	set(SPDLOG_INCLUDE_DIRS ${SPDLOG_INCLUDE_DIR})
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
		message("Downloading ${CATCH2_NAME}")
		file(DOWNLOAD "${CATCH2_URL}" "${CATCH2_DOWNLOAD_PATH}")
	endif()

	# Extract
	if (NOT EXISTS "${CATCH2_EXTRACTED_FILE}/${CATCH2_NAME}")
		execute_process(
			COMMAND ${CMAKE_COMMAND} -E tar xzf ${CATCH2_DOWNLOAD_PATH}
			WORKING_DIRECTORY ${CATCH2_EXTRACTED_FILE})
	endif()

	#add_definitions( -DCATCH_CONFIG_ENABLE_BENCHMARKING )

	set(CATCH2_DIR ${MODULES_PATH}/${CATCH2_NAME})
	add_subdirectory(${CATCH2_DIR}  ${PROJECT_BINARY_DIR}/modules/${CATCH2_NAME})

	# Output variables
	set(CATCH2_LIBRARY Catch2)
	get_filename_component(CATCH2_INCLUDE_DIR ${CATCH2_DIR}/include ABSOLUTE)
	set(CATCH2_LIBRARIES ${CATCH2_LIBRARY})
	set(CATCH2_INCLUDE_DIRS ${CATCH2_INCLUDE_DIR})
endif()

