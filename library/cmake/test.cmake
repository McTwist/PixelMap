# Testing area

set(COMMIT "454a96e3c795a67fd718e80855587fa0d3b7be8b")

cmake_host_system_information(RESULT nproc QUERY NUMBER_OF_LOGICAL_CORES)

# Ensure availability of tests
add_test(NAME build-tests COMMAND "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target tests --parallel ${nproc})

# Unit tests
add_test(NAME unit-tests COMMAND $<TARGET_FILE:tests>)

# Dependencies
set_tests_properties(build-tests PROPERTIES FIXTURES_SETUP unit)
set_tests_properties(unit-tests PROPERTIES FIXTURES_REQUIRED unit)

add_custom_target(integration)

# Ensure availability of cli
add_test(NAME build-pixelmapcli COMMAND "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target pixelmapcli --parallel ${nproc})
add_test(NAME integration-download COMMAND "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target integration)

set_tests_properties(build-pixelmapcli PROPERTIES DEPENDS unit)
set_tests_properties(integration-download PROPERTIES DEPENDS build-pixelmapcli)

set_tests_properties(build-pixelmapcli PROPERTIES FIXTURES_SETUP integration)
set_tests_properties(integration-download PROPERTIES FIXTURES_SETUP integration)

# Find all programs used
find_program(BASH_EXECUTABLE NAMES bash REQUIRED)
set(PIXELMAPCLI $<TARGET_FILE:pixelmapcli>)
find_program(COMPARE NAMES compare REQUIRED)
find_file(INTEGRATION_SH NAMES integration.sh PATHS tests)

# Integration tests
include(ExternalData)
set(ExternalData_URL_TEMPLATES "ExternalDataCustomScript://encoding/https://git.aposoc.net/McTwist/PixelMap_data/raw/commit/${COMMIT}/gen/%(algo)/%(hash)")
set(ExternalData_CUSTOM_SCRIPT_encoding "${CMAKE_CURRENT_LIST_DIR}/DownloadTest.cmake")

file(GLOB test_worlds "test_data/*.tar.sha256")
foreach(path ${test_worlds})
	cmake_path(GET path FILENAME file)
	cmake_path(GET file STEM name)
	set(TEST_NAME "test_${name}")
	ExternalData_Add_Test("${TEST_NAME}" NAME ${name}
		COMMAND
			${CMAKE_COMMAND} -E env PIXELMAPCLI=${PIXELMAPCLI} COMPARE=${COMPARE}
			"${BASH_EXECUTABLE}" "${INTEGRATION_SH}" DATA{test_data/${name}.tar} DATA{test_data/${name}.png})
	ExternalData_Add_Target("${TEST_NAME}")
	add_dependencies(integration "${TEST_NAME}")
	set_tests_properties(${name} PROPERTIES FIXTURES_REQUIRED integration)
	unset(name)
	unset(file)
endforeach()

