
# Force multithreaded
if (MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE /ENTRY:mainCRTStartup")
endif()

set(PIXELMAPCLI_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set(PIXELMAPCLI_HEADER
	"${PIXELMAPCLI_INCLUDE_DIR}/console.hpp"
	"${PIXELMAPCLI_INCLUDE_DIR}/events.hpp"
	"${PIXELMAPCLI_INCLUDE_DIR}/profiler.hpp"
	"${PIXELMAPCLI_INCLUDE_DIR}/programoptions.hpp"
	)

set(PIXELMAPCLI_SRC
	"console.cpp"
	"events.cpp"
	"profiler.cpp"
	"programoptions.cpp"
	"main.cpp"
	)

# Get necessary variables
get_directory_property(PIXELMAP_INCLUDE_DIR DIRECTORY ${PIXELMAP_DIR}/src DEFINITION PIXELMAP_INCLUDE_DIR)
get_directory_property(GLM_INCLUDE_DIR DIRECTORY ${PIXELMAP_DIR}/src DEFINITION GLM_INCLUDE_DIR)
get_directory_property(SPDLOG_INCLUDE_DIR DIRECTORY ${PIXELMAP_DIR}/src DEFINITION SPDLOG_INCLUDE_DIR)

# Add include directories from libraries
include_directories(${PIXELMAP_INCLUDE_DIR} ${SPDLOG_INCLUDE_DIR} ${GLM_INCLUDE_DIR})

# Create the executable
add_executable(pixelmapcli ${PIXELMAPCLI_HEADER} ${PIXELMAPCLI_SRC})
target_include_directories(pixelmapcli PUBLIC
	$<BUILD_INTERFACE:${PIXELMAPCLI_INCLUDE_DIR}>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)

# Link everything together
target_link_libraries(pixelmapcli pixelmap)

if(${PIXELMAP_STRIP})
	add_custom_command(
		TARGET pixelmapcli POST_BUILD
		COMMAND strip "$<TARGET_FILE:pixelmapcli>"
		VERBATIM
	)
endif()

