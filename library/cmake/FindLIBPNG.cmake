# FindLIBPNG
# LIBPNG_INCLUDE_DIRS - include directories for libpng
# LIBPNG_LIBRARIES - libraries to link against libpng
# LIBPNG_FOUND - true if libpng has been found and can be used

set(LIBPNG_SEARCH_PATHS
	~/Library/Frameworks
	/Library/Frameworks
	/usr/local
	/usr
	/sw # Fink
	/opt/local # DarwinPorts
	/opt/csw # Blastwave
	/opt
)

find_path(LIBPNG_INCLUDE_DIR png.h
	HINTS
	PATHS ${LIBPNG_SEARCH_PATHS}
)

find_library(LIBPNG_LIBRARY NAMES png libpng
	HINTS
	$ENV{EXPATDIR}
	PATH_SUFFIXES lib lib/x86 lib64 lib/x64
	PATHS ${LIBPNG_SEARCH_PATHS}
)

set(LIBPNG_INCLUDE_DIRS ${LIBPNG_INCLUDE_DIR})
set(LIBPNG_LIBRARIES ${LIBPNG_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LIBPNG REQUIRED_VARS LIBPNG_INCLUDE_DIRS LIBPNG_LIBRARIES)
