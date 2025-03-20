

set(PIXELMAPCLI_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set(PIXELMAPCLI_HEADER
	"${PIXELMAPCLI_INCLUDE_DIR}/console.hpp"
	"${PIXELMAPCLI_INCLUDE_DIR}/events.hpp"
	"${PIXELMAPCLI_INCLUDE_DIR}/programoptions.hpp"
	)

set(PIXELMAPCLI_SRC
	"console.cpp"
	"events.cpp"
	"programoptions.cpp"
	"main.cpp"
	)

if(PIXELMAP_PROFILE)
	list(APPEND PIXELMAPCLI_HEADER
		"${PIXELMAPCLI_INCLUDE_DIR}/profiler.hpp")

	list(APPEND PIXELMAPCLI_SRC
		"profiler.cpp")
endif()

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

set_target_properties(pixelmapcli PROPERTIES ENABLE_EXPORTS 1)

target_link_options(pixelmapcli PRIVATE
	$<$<CXX_COMPILER_ID:MSVC>:/SUBSYSTEM:CONSOLE /ENTRY:mainCRTStartup>
)

if(PIXELMAP_PROFILE)
	target_compile_definitions(pixelmapcli PRIVATE ENABLE_PROFILER)
endif()

# Link everything together
target_link_libraries(pixelmapcli pixelmap)

if(PIXELMAP_STRIP)
	add_custom_command(
		TARGET pixelmapcli POST_BUILD
		COMMAND strip "$<TARGET_FILE:pixelmapcli>"
		VERBATIM
	)
endif()

