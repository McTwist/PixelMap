# Based on the FindAssimp.cmake scipt
# - Try to find Imgui
# Once done this will define
#
#  IMGUI_FOUND - system has Imgui
#  IMGUI_INCLUDE_DIR - the Imgui include directory
#  IMGUI_LIBRARY - Link these to use Imgui

set(_imgui_SEARCH_DIRS
	~/Library/Frameworks
	/Library/Frameworks
	/usr/local
	/usr
	/sw # Fink
	/opt/local # DarwinPorts
	/opt/csw # Blastwave
	/opt
)

find_path(IMGUI_INCLUDE_DIR imgui.h
	PATH_SUFFIXES include
	PATHS ${_imgui_SEARCH_DIRS}
)

find_library(IMGUI_LIBRARY NAMES imgui
	PATH_SUFFIXES lib lib/x86 lib64 lib/x64
	PATHS ${_imgui_SEARCH_DIRS}
)

set(IMGUI_FOUND "NO")
if(IMGUI_INCLUDE_DIR AND IMGUI_LIBRARY)
	set(IMGUI_FOUND "YES")
endif(IMGUI_INCLUDE_DIR AND IMGUI_LIBRARY)

mark_as_advanced(IMGUI_LIBRARY IMGUI_INCLUDE_DIR)

