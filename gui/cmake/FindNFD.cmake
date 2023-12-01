# Based on the FindAssimp.cmake scipt
# - Try to find NFD
# Once done this will define
#
#  NFD_FOUND - system has NFD
#  NFD_INCLUDE_DIR - the NFD include directory
#  NFD_LIBRARY - Link these to use NFD

set(_nfd_SEARCH_DIRS
	~/Library/Frameworks
	/Library/Frameworks
	/usr/local
	/usr
	/sw # Fink
	/opt/local # DarwinPorts
	/opt/csw # Blastwave
	/opt
)

find_path(NFD_INCLUDE_DIR nfd.h
	PATH_SUFFIXES include
	PATHS ${_nfd_SEARCH_DIRS}
)

find_library(NFD_LIBRARY NAMES nfd
	PATH_SUFFIXES lib lib/x86 lib64 lib/x64
	PATHS ${_nfd_SEARCH_DIRS}
)

set(NFD_FOUND "NO")
if(NFD_INCLUDE_DIR AND NFD_LIBRARY)
	set(NFD_FOUND "YES")
endif(NFD_INCLUDE_DIR AND NFD_LIBRARY)

mark_as_advanced(NFD_LIBRARY NFD_INCLUDE_DIR)

