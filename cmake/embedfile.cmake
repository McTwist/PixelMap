
# https://beesbuzz.biz/code/4399-Embedding-binary-resources-with-CMake-and-C-11
# https://jonathanhamberg.com/post/cmake-file-embedding/

if (NOT DEFINED EMBED_INCLUDE_DIR)
	set(EMBED_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/resources)
endif()

function(EmbedFile file out_cpp out_hpp)
	cmake_parse_arguments(
		PARSE_ARGV 3 arg
		""
		"IDENTIFIER"
		"DEPENDS"
	)

	cmake_path(ABSOLUTE_PATH file)
	if (arg_IDENTIFIER)
		set(name ${arg_IDENTIFIER})
	else()
		cmake_path(RELATIVE_PATH file OUTPUT_VARIABLE name)
	endif()

	EmbedFile_Internal(${file} ${name} ${out_cpp} ${out_hpp})
	set(${out_cpp} ${${out_cpp}} PARENT_SCOPE)
	set(${out_hpp} ${${out_hpp}} PARENT_SCOPE)

	if(arg_DEPENDS)
		set(depends DEPENDS ${arg_DEPENDS})
	endif()

	add_custom_command(
		OUTPUT ${${out_cpp}} ${${out_hpp}}
		COMMAND ${CMAKE_COMMAND}
			-DEMBED_INCLUDE_DIR=${EMBED_INCLUDE_DIR}
			-DEMBED_FILE_GENERATE=ON
			-DEMBED_FILE_GENERATE_NAME=${name}
			-DEMBED_FILE_GENERATE_FILE=${file}
			-P ${CMAKE_SOURCE_DIR}/cmake/embedfile.cmake
		MAIN_DEPENDENCY ${file}
		${depends}
	)
endfunction()

function(EmbedFile_Internal file name cpp_output hpp_output)
	file(READ ${file} content HEX)

	string(MAKE_C_IDENTIFIER ${name} c_name)

	string(REGEX MATCHALL "([A-Fa-f0-9][A-Fa-f0-9])" SEPARATED_HEX ${content})

	set(i 0)
	foreach (hex IN LISTS SEPARATED_HEX)
		string(APPEND output "0x${hex},")
		math(EXPR i "${i}+1")
		if (i GREATER 16)
			string(APPEND output "\n    ")
			set(i 0)
		endif()
	endforeach()

	set(output_cpp "\
#include <cstdint>
uint8_t ${c_name}_data[] = {
	${output}
}\;
uint64_t ${c_name}_size = sizeof(${c_name}_data)\;
")

	set(output_hpp "\
#pragma once
#include <cstdint>
extern uint8_t ${c_name}_data[]\;
extern uint64_t ${c_name}_size\;
")

	file(MAKE_DIRECTORY ${EMBED_INCLUDE_DIR})

	file(WRITE ${EMBED_INCLUDE_DIR}/${c_name}.cpp ${output_cpp})
	file(WRITE ${EMBED_INCLUDE_DIR}/${c_name}.hpp ${output_hpp})

	set(${cpp_output} ${EMBED_INCLUDE_DIR}/${c_name}.cpp PARENT_SCOPE)
	set(${hpp_output} ${EMBED_INCLUDE_DIR}/${c_name}.hpp PARENT_SCOPE)
endfunction()

if (EMBED_FILE_GENERATE)
	EmbedFile_Internal(${EMBED_FILE_GENERATE_FILE} ${EMBED_FILE_GENERATE_NAME} _ _)
endif()
