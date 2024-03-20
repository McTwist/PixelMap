
# Set paths
set(DOWNLOAD_PATH ${PROJECT_BINARY_DIR}/downloads)
set(MODULES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../modules)
file(MAKE_DIRECTORY ${DOWNLOAD_PATH})
file(MAKE_DIRECTORY ${MODULES_PATH})

message(STATUS "Preparing dependencies for gui...")

set(SDL2_VERSION 2.28.5)
set(NFD_VERSION 1.1.0)
set(IMGUI_VERSION 1.89.9)

# OpenGL
set(OpenGL_GL_PREFERENCE LEGACY)

# Load packages
find_package(OpenGL REQUIRED)
find_package(SDL2 CONFIGS)
find_package(Imgui)
find_package(NFD)

if (NOT SDL2_FOUND)
	set(SDL2_NAME "SDL-release-${SDL2_VERSION}")
	# Download
	set(SDL2_URL "https://github.com/libsdl-org/SDL/archive/refs/tags/release-${SDL2_VERSION}.tar.gz")
	set(SDL2_DOWNLOAD_PATH ${DOWNLOAD_PATH}/${SDL2_NAME}.tar.gz)
	set(SDL2_EXTRACTED_FILE ${MODULES_PATH})

	if (NOT EXISTS "${SDL2_DOWNLOAD_PATH}")
		message(STATUS "Downloading ${SDL2_NAME}")
		file(DOWNLOAD "${SDL2_URL}" "${SDL2_DOWNLOAD_PATH}")
	endif()

	# Extract
	if (NOT EXISTS "${SDL2_EXTRACTED_FILE}/${SDL2_NAME}")
		execute_process(
			COMMAND ${CMAKE_COMMAND} -E tar xzf ${SDL2_DOWNLOAD_PATH}
			WORKING_DIRECTORY ${SDL2_EXTRACTED_FILE})
	endif()

	set(SDL_TEST OFF)
	set(SDL_SHARED ON)
	set(SDL_STATIC OFF)
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

	set(SDL2_DIR "${MODULES_PATH}/${SDL2_NAME}")
	add_subdirectory("${SDL2_DIR}" "${PROJECT_BINARY_DIR}/modules/${SDL2_NAME}")

	# Output variables
	set(SDL2_LIBRARY SDL2-static)
	set(SDL2_LIBRARIES ${SDL2_LIBRARY})
	get_filename_component(SDL2_INCLUDE_DIR "${SDL2_DIR}/include" ABSOLUTE CACHE)
	set(SDL2_INCLUDE_DIRS "${SDL2_INCLUDE_DIR}" CACHE PATH "sdl2 include dir" FORCE)
	set(SDL2_DEPEND true)
	#set_property(TARGET ${SDL2_LIBRARY} PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

if(NOT NFD_FOUND)
	set(NFD_NAME "nativefiledialog-extended-${NFD_VERSION}")
	# Download
	set(NFD_URL "https://github.com/btzy/nativefiledialog-extended/archive/refs/tags/v${NFD_VERSION}.tar.gz")
	set(NFD_DOWNLOAD_PATH ${DOWNLOAD_PATH}/${NFD_NAME}.tar.gz)
	set(NFD_EXTRACTED_FILE ${MODULES_PATH})

	if (NOT EXISTS "${NFD_DOWNLOAD_PATH}")
		message(STATUS "Downloading ${NFD_NAME}")
		file(DOWNLOAD "${NFD_URL}" "${NFD_DOWNLOAD_PATH}")
	endif()

	# Extract
	if (NOT EXISTS "${NFD_EXTRACTED_FILE}/${NFD_NAME}")
		execute_process(
			COMMAND ${CMAKE_COMMAND} -E tar xzf ${NFD_DOWNLOAD_PATH}
			WORKING_DIRECTORY ${NFD_EXTRACTED_FILE})
	endif()

	add_definitions(-DNFD_BUILD_TESTS=OFF -DNFD_INSTALL=OFF)

	set(NFD_DIR "${MODULES_PATH}/${NFD_NAME}")
	add_subdirectory("${NFD_DIR}" "${PROJECT_BINARY_DIR}/modules/${NFD_NAME}")

	# Output variables
	set(NFD_LIBRARY nfd)
	set(NFD_LIBRARIES ${NFD_LIBRARY})
	get_filename_component(NFD_INCLUDE_DIR "${NFD_DIR}/src/include" ABSOLUTE CACHE)
	set(NFD_INCLUDE_DIRS "${NFD_INCLUDE_DIR}" CACHE PATH "nfd include dirs" FORCE)
endif()

if(NOT IMGUI_FOUND)
	set(IMGUI_NAME "imgui-${IMGUI_VERSION}")
	# Download
	set(IMGUI_URL "https://github.com/ocornut/imgui/archive/refs/tags/v${IMGUI_VERSION}.tar.gz")
	set(IMGUI_DOWNLOAD_PATH ${DOWNLOAD_PATH}/${IMGUI_NAME}.tar.gz)
	set(IMGUI_EXTRACTED_FILE ${MODULES_PATH})

	if (NOT EXISTS "${IMGUI_DOWNLOAD_PATH}")
		message(STATUS "Downloading ${IMGUI_NAME}")
		file(DOWNLOAD "${IMGUI_URL}" "${IMGUI_DOWNLOAD_PATH}")
	endif()

	# Extract
	if (NOT EXISTS "${IMGUI_EXTRACTED_FILE}/${IMGUI_NAME}")
		execute_process(
			COMMAND ${CMAKE_COMMAND} -E tar xzf ${IMGUI_DOWNLOAD_PATH}
			WORKING_DIRECTORY ${IMGUI_EXTRACTED_FILE})
	endif()

	set(IMGUI_DIR "${MODULES_PATH}/${IMGUI_NAME}")
	# Build include the custom one
	add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src/imgui" "${PROJECT_BINARY_DIR}/modules/${IMGUI_NAME}")

	# Output variables
	set(IMGUI_LIBRARY imgui)
	set(IMGUI_LIBRARIES ${IMGUI_LIBRARY})
	get_filename_component(IMGUI_INCLUDE_DIR "${IMGUI_DIR}" ABSOLUTE CACHE)
	set(IMGUI_INCLUDE_DIRS "${IMGUI_INCLUDE_DIR}" CACHE PATH "imgui include dirs" FORCE)
endif()

if(OPENGL_FOUND)
include_directories(${OPENGL_INCLUDE_DIRS})    
endif()

# Load packages
set(PIXELMAP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../library")
include_directories(${PIXELMAP_DIR})

