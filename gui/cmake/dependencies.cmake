
message(STATUS "Preparing dependencies for gui...")

# Override policies
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

set(SDL2_VERSION 2.30.6)
set(IMGUI_VERSION 1.91.0)
set(NFD_VERSION 1.2.1)

# OpenGL
set(OpenGL_GL_PREFERENCE LEGACY)

# Load packages
find_package(OpenGL REQUIRED)

# SDL2
FetchContent_Declare(
	SDL2
	GIT_REPOSITORY "https://github.com/libsdl-org/SDL.git"
	GIT_TAG "release-${SDL2_VERSION}"
	FIND_PACKAGE_ARGS CONFIG
)

set(SDL_TEST OFF)
set(SDL_SHARED OFF)
set(SDL_STATIC ON)
set(SDL_TEST_LIBRARY OFF)
set(SDL2_DISABLE_INSTALL ON)
set(SDL2_DISABLE_UNINSTALL ON)
set(SDL2_DISABLE_SDL2MAIN ON CACHE BOOL "Disable building/installation of SDL2main" FORCE)
set(SDL_STATIC_PIC ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
# Subsystems
set(SDL_AUDIO OFF)
set(SDL_JOYSTICK OFF)
set(SDL_HAPTIC OFF)
set(SDL_HIDAPI OFF)
set(SDL_POWER OFF)
set(SDL_SENSOR OFF)
FetchContent_MakeAvailable(SDL2)
# Need special handling if built from source
if ("${SDL2_LIBRARIES}" STREQUAL "")
	set(SDL2_LIBRARY SDL2-static)
	set(SDL2_LIBRARIES ${SDL2_LIBRARY})
	get_filename_component(SDL2_INCLUDE_DIR "${sdl2_SOURCE_DIR}" ABSOLUTE CACHE)
	set(SDL2_INCLUDE_DIRS "${SDL2_INCLUDE_DIR}" CACHE PATH "SDL2 include dirs" FORCE)
endif()

# Imgui
FetchContent_Declare(
	Imgui
	GIT_REPOSITORY "https://github.com/ocornut/imgui.git"
	GIT_TAG "v${IMGUI_VERSION}"
	CONFIGURE_COMMAND ""
	BUILD_COMMAND ""
)

FetchContent_MakeAvailable(Imgui)
# Note: Does not have a CMakelists.txt, so we set everything together that we actually need, reducing dependencies
set(IMGUI_DIR "${imgui_SOURCE_DIR}")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src/imgui" "${PROJECT_BINARY_DIR}/modules/${IMGUI_NAME}")
set(IMGUI_LIBRARY imgui)
set(IMGUI_LIBRARIES ${IMGUI_LIBRARY})
get_filename_component(IMGUI_INCLUDE_DIR "${IMGUI_DIR}" ABSOLUTE CACHE)
set(IMGUI_INCLUDE_DIRS "${IMGUI_INCLUDE_DIR}" CACHE PATH "imgui include dirs" FORCE)

# NFD
FetchContent_Declare(
	NFD
	GIT_REPOSITORY "https://github.com/btzy/nativefiledialog-extended.git"
	GIT_TAG "v${NFD_VERSION}"
)

add_definitions(-DNFD_BUILD_TESTS=OFF -DNFD_INSTALL=OFF)
FetchContent_MakeAvailable(NFD)
set(NFD_LIBRARY nfd)
set(NFD_LIBRARIES ${NFD_LIBRARY})

if(OPENGL_FOUND)
include_directories(${OPENGL_INCLUDE_DIRS})
endif()

# Load packages
set(PIXELMAP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../library")
include_directories(${PIXELMAP_DIR})

