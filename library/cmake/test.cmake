# Testing area

cmake_host_system_information(RESULT nproc QUERY NUMBER_OF_LOGICAL_CORES)

# Ensure availability of tests
add_test(NAME build-tests COMMAND "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target tests --parallel ${nproc})

# Unit tests
add_test(NAME unit-tests COMMAND $<TARGET_FILE:tests>)

# Dependencies
set_tests_properties(build-tests PROPERTIES FIXTURES_SETUP built)
set_tests_properties(unit-tests PROPERTIES FIXTURES_REQUIRED built)
