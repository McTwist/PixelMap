# Testing area

set(COMMIT "eb6b8ae63fa1f6a7fd88823cdbcd08b674ec11fb")
set(SPECS_WORLD "je_1_21_5_full")

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
find_file(IMAGE_SH NAMES image.sh PATHS scripts)
find_file(TAR_SH NAMES tar.sh PATHS scripts)

# Integration tests
include(ExternalData)
set(ExternalData_URL_TEMPLATES "ExternalDataCustomScript://encoding/https://git.aposoc.net/McTwist/PixelMap_data/raw/commit/${COMMIT}/gen/%(algo)/%(hash)")
set(ExternalData_CUSTOM_SCRIPT_encoding "${CMAKE_CURRENT_LIST_DIR}/DownloadTest.cmake")

# Prepare integration test
function(add_render_test world file script)
	cmake_path(GET file STEM name)
	set(TEST_NAME "test_${name}")
	ExternalData_Add_Test("${TEST_NAME}" NAME ${name}
		COMMAND
			${CMAKE_COMMAND} -E env PIXELMAPCLI=${PIXELMAPCLI} COMPARE=${COMPARE}
			"${BASH_EXECUTABLE}" "${script}" DATA{test_data/worlds/${world}.tar} DATA{test_data/${file}} ${ARGN})
	ExternalData_Add_Target("${TEST_NAME}")
	add_dependencies(integration "${TEST_NAME}")
	set_tests_properties(${name} PROPERTIES FIXTURES_REQUIRED integration)
endfunction()

# Prepare version test
macro(add_version_test name)
	add_render_test("${name}" "images/${name}.png" ${IMAGE_SH})
endmacro()

# Prepare image based specs test
macro(add_specs_test name)
	add_render_test("${SPECS_WORLD}" "specs/${SPECS_WORLD}_${name}.png" ${IMAGE_SH} ${ARGN})
endmacro()

# Prepare folder based specs test
macro(add_specs_test_tar name render)
	add_render_test("${SPECS_WORLD}" "specs/${SPECS_WORLD}_${name}.tar" ${TAR_SH} ${render})
endmacro()

# Versions
file(GLOB test_worlds "test_data/worlds/*.tar.sha256")
foreach(path ${test_worlds})
	cmake_path(GET path STEM name)
	add_version_test(${name})
	unset(name)
endforeach()

# Rendering layers
add_specs_test("layer_cave" --cave)
add_specs_test("layer_gradient" --gradient)
add_specs_test("layer_heightline4" --heightline 4)
add_specs_test("layer_night" --night)
add_specs_test("layer_opaque" --opaque)
add_specs_test("layer_slice32" --slice 32)
add_specs_test("layer_gray" -m gray)
add_specs_test("layer_color" -m color)

# Dimensions
add_specs_test("dim_nether" -d -1 --cave)
add_specs_test("dim_end" -d 1)

# Rendering blends
add_specs_test("blend_normal" --blend normal)
add_specs_test("blend_multiply" --blend multiply)
add_specs_test("blend_screen" --blend screen)
add_specs_test("blend_overlay" --blend overlay)
add_specs_test("blend_darken" --blend darken)
add_specs_test("blend_lighten" --blend lighten)
add_specs_test("blend_color_dodge" --blend color_dodge)
add_specs_test("blend_hard_light" --blend hard_light)
add_specs_test("blend_soft_light" --blend soft_light)
add_specs_test("blend_difference" --blend difference)
add_specs_test("blend_exclusion" --blend exclusion)
add_specs_test("blend_hue" --blend hue)
add_specs_test("blend_saturation" --blend saturation)
add_specs_test("blend_color" --blend color)
add_specs_test("blend_luminosity" --blend luminosity)

# Rendering output
add_specs_test_tar("map" map)
add_specs_test_tar("web" web)


