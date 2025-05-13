
message(STATUS "Preparing dependencies for cli...")

set(SPDMON_VERSION 9ca6d4325c70d39da67867aae13f3729b942a000) # Has no tags

# spdmon
if(PIXELMAPCLI_USE_SPDMON)
	FetchContent_Declare(
		SPDMON
		GIT_REPOSITORY "https://github.com/pfmephisto/spdmon.git"
		GIT_TAG "${SPDMON_VERSION}"
		EXCLUDE_FROM_ALL
		CONFIGURE_COMMAND ""
		BUILD_COMMAND ""
	)

	FetchContent_Populate(SPDMON)
	# Note: CMakelists.txt is broken, set everything manually
	set(SPDMON_DIR "${spdmon_SOURCE_DIR}")
	# Patch bug in spdmon
	execute_process(
		COMMAND git reset --hard
		WORKING_DIRECTORY "${SPDMON_DIR}")
	execute_process(
		COMMAND git apply "${CMAKE_CURRENT_LIST_DIR}/spdmon.patch"
		WORKING_DIRECTORY "${SPDMON_DIR}")
	get_filename_component(SPDMON_INCLUDE_DIR "${SPDMON_DIR}/include" ABSOLUTE CACHE)
	set(SPDMON_INCLUDE_DIRS "${SPDMON_INCLUDE_DIR}" CACHE PATH "spdmon include dirs" FORCE)
	set(SPDMON_LICENSE_FILE "${spdmon_SOURCE_DIR}/LICENSE")
endif()

# Load packages
set(PIXELMAP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../library)
include_directories(${PIXELMAP_DIR})
