# https://github.com/madler/zlib/issues/759#issuecomment-1384244323
macro(FetchContent_MakeAvailableExcludeFromAll)
	foreach(contentName IN ITEMS ${ARGV})
		string(TOLOWER ${contentName} contentNameLower)
		FetchContent_GetProperties(${contentName})
		if(NOT ${contentNameLower}_POPULATED)
			FetchContent_Populate(${contentName})
			if(EXISTS ${${contentNameLower}_SOURCE_DIR}/CMakeLists.txt)
				add_subdirectory(${${contentNameLower}_SOURCE_DIR}
					${${contentNameLower}_BINARY_DIR} EXCLUDE_FROM_ALL)
			endif()
		endif()
	endforeach()
endmacro()
