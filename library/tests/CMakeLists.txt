

set(TESTS_SRC
	"tests.cpp"
	"tests-color.cpp"
	"tests-endianess.cpp"
	"tests-eventhandler.cpp"
	"tests-leveldb.cpp"
	"tests-nbt.cpp"
	"tests-nibble.cpp"
	"tests-utility.cpp"
	)

# Add include directories from libraries
include_directories(${PIXELMAP_INCLUDE_DIR} ${CATCH2_INCLUDE_DIR} ${GLM_INCLUDE_DIR})

if(MSVC)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE /ENTRY:mainCRTStartup")
endif()

# Create the executable
add_executable(tests ${TESTS_SRC})
add_dependencies(tests pixelmap)

# Link everything together
target_link_libraries(tests pixelmap ${CATCH2_LIBRARY})
