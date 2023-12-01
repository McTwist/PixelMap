# FindSPDLOG
# SPDLOG_INCLUDE_DIRS - include directories for spdlog
# SPDLOG_LIBRARIES - libraries to link against spdlog
# SPDLOG_FOUND - true if spdlog has been found and can be used

set(SPDLOG_SEARCH_PATHS
	~/Library/Frameworks
	/Library/Frameworks
	/usr/local
	/usr
	/sw # Fink
	/opt/local # DarwinPorts
	/opt/csw # Blastwave
	/opt
)

find_path(SPDLOG_INCLUDE_DIR spdlog/spdlog.h
	HINTS
	PATHS ${SPDLOG_SEARCH_PATHS}
)

find_library(SPDLOG_LIBRARY NAMES spdlog SPDLOG
	HINTS
	$ENV{EXPATDIR}
	PATH_SUFFIXES lib lib/x86 lib64 lib/x64
	PATHS ${SPDLOG_SEARCH_PATHS}
)

set(SPDLOG_INCLUDE_DIRS ${SPDLOG_INCLUDE_DIR})
set(SPDLOG_LIBRARIES ${SPDLOG_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SPDLOG REQUIRED_VARS SPDLOG_INCLUDE_DIRS SPDLOG_LIBRARIES)
