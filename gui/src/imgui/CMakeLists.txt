
# Force multithreaded
if (MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
endif()

set(IMGUI_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include/imgui")

set(IMGUI_HEADER
	"${IMGUI_DIR}/imgui.h"
	"${IMGUI_DIR}/imconfig.h"
	"${IMGUI_DIR}/imgui_internal.h"
	"${IMGUI_DIR}/backends/imgui_impl_sdl2.h"
	"${IMGUI_DIR}/backends/imgui_impl_opengl3.h"
	"${IMGUI_DIR}/backends/imgui_impl_opengl3_loader.h")

set(IMGUI_SRC
	"${IMGUI_DIR}/imgui.cpp"
	"${IMGUI_DIR}/imgui_draw.cpp"
	"${IMGUI_DIR}/imgui_tables.cpp"
	"${IMGUI_DIR}/imgui_widgets.cpp"
	"${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp"
	"${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp")

# Add include directories from libraries
include_directories(${SDL2_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS} ${IMGUI_DIR})

# Create the static library
add_library(imgui STATIC ${IMGUI_HEADER} ${IMGUI_SRC})
#add_dependencies(imgui ${SDL_LIBRARIES} ${OPENGL_LIBRARIES})
target_include_directories(imgui PUBLIC
	$<BUILD_INTERFACE:${IMGUI_INCLUDE_DIR}>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)

# Link everything together
target_link_libraries(imgui ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES} ${CMAKE_DL_LIBS})

