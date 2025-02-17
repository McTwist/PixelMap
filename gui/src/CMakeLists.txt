
# Force multithreaded
if (MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
endif()

set(PIXELMAPIM_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")

set(PIXELMAPIM_HEADER
	"${PIXELMAPIM_INCLUDE_DIR}/gui.hpp"
	"${PIXELMAPIM_INCLUDE_DIR}/icon.hpp"
	"${PIXELMAPIM_INCLUDE_DIR}/imgui/imgui_custom.h"
	)

set(PIXELMAPIM_SRC
	"main.cpp"
	"gui.cpp"
	"imgui/imgui_custom.cpp"
	)

# Get necessary variables
get_directory_property(PIXELMAP_INCLUDE_DIR DIRECTORY ${PIXELMAP_DIR}/src DEFINITION PIXELMAP_INCLUDE_DIR)
get_directory_property(GLM_INCLUDE_DIRS DIRECTORY ${PIXELMAP_DIR}/src DEFINITION GLM_INCLUDE_DIRS)
get_directory_property(SPDLOG_INCLUDE_DIRS DIRECTORY ${PIXELMAP_DIR}/src DEFINITION SPDLOG_INCLUDE_DIRS)

EmbedFile("resource/DroidSans.ttf" DROIDSANS_CPP DROIDSANS_HPP)

# Add include directories from libraries
include_directories(${PIXELMAP_INCLUDE_DIR} ${SPDLOG_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS} ${NFD_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS} ${IMGUI_INCLUDE_DIRS})

# Create the executable
add_executable(pixelmapim WIN32 ${PIXELMAPIM_HEADER} ${PIXELMAPIM_SRC})
target_include_directories(pixelmapim PUBLIC
    $<BUILD_INTERFACE:${PIXELMAPIM_INCLUDE_DIR}>
	$<BUILD_INTERFACE:${EMBED_INCLUDE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)

target_sources(pixelmapim PRIVATE ${DROIDSANS_HPP} ${DROIDSANS_CPP})

# Link everything together
target_link_libraries(pixelmapim
	pixelmap
	${IMGUI_LIBRARIES}
	${NFD_LIBRARIES})

if(${PIXELMAP_STRIP})
	add_custom_command(
		TARGET pixelmapim POST_BUILD
		COMMAND strip "$<TARGET_FILE:pixelmapim>"
		VERBATIM
	)
endif()
