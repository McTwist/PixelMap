
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
get_directory_property(GLM_INCLUDE_DIR DIRECTORY ${PIXELMAP_DIR}/src DEFINITION GLM_INCLUDE_DIR)
get_directory_property(PIXELMAP_LIBRARIES DIRECTORY ${PIXELMAP_DIR}/src DEFINITION PIXELMAP_LIBRARIES)

# Add include directories from libraries
include_directories(${PIXELMAP_INCLUDE_DIR} ${GLM_INCLUDE_DIRS} ${NFD_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS} ${IMGUI_INCLUDE_DIRS})

set_target_properties(pixelmap PROPERTIES DEFINE_SYMBOL PIXELMAP_DLL)

# Create the executable
add_executable(pixelmapim ${PIXELMAPIM_HEADER} ${PIXELMAPIM_SRC})
add_dependencies(pixelmapim pixelmap ${IMGUI_LIBRARIES} ${NFD_LIBRARIES})
target_include_directories(pixelmapim PUBLIC
    $<BUILD_INTERFACE:${PIXELMAPIM_INCLUDE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)

# Link everything together
target_link_libraries(pixelmapim pixelmap ${IMGUI_LIBRARIES} ${NFD_LIBRARIES})
