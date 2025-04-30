# - Generated command.
#
# A command will be executed both when generating
# and when building. It will make sure that dependencies
# are followed.
#
#    GenerateCommand(
#        OUTPUT <output>
#        [DEPENDS [<depend>...]]
#        [BYPRODUCTS [<byproduct>...]]
#        [COMMENT <message>]
#        TERMINATE
#        [<cmd>...]
#	)
#    GenerateCommand(
#        TARGET <target>
#        [DEPENDS [<depend>...]]
#        [BYPRODUCTS [<byproduct>...]]
#        [COMMENT <message>]
#        TERMINATE
#        [<cmd>...]
#	)
#
# TERMINATE is required to separate <cmd> from all other
# arguments.

function(GenerateCommand)
	cmake_parse_arguments(
		PARSE_ARGV 0 arg
		"TERMINATE"
		"COMMENT"
		"OUTPUT;TARGET;DEPENDS;BYPRODUCTS"
	)

	if(arg_OUTPUT AND arg_TARGET)
		message(FATAL_ERROR "Cannot have both OUTPUT and TARGET")
	endif()
	if(NOT arg_OUTPUT AND NOT arg_TARGET)
		message(FATAL_ERROR "Require either OUTPUT and TARGET")
	endif()

	set(command "")
	if(arg_OUTPUT)
		list(APPEND command OUTPUT ${arg_OUTPUT})
	endif()
	if(arg_TARGET)
		list(APPEND command TARGET ${arg_TARGET})
	endif()
	if(arg_COMMENT)
		list(APPEND command COMMENT ${arg_COMMENT})
	endif()
	if(arg_DEPENDS)
		list(APPEND command DEPENDS ${arg_DEPENDS})
	endif()
	if(arg_BYPRODUCTS)
		list(APPEND command BYPRODUCTS ${arg_BYPRODUCTS})
	endif()
	cmake_parse_arguments(
		cmd
		""
		""
		"WORKING_DIRECTORY"
		${arg_UNPARSED_ARGUMENTS}
	)
	if (cmd_WORKING_DIRECTORY)
		list(APPEND command WORKING_DIRECTORY "${cmd_WORKING_DIRECTORY}")
	endif()

	GenerateCommand_Internal(${arg_UNPARSED_ARGUMENTS})

	# Required in order for it to work properly
	string(REPLACE "\\" "\\\\;" args "${arg_UNPARSED_ARGUMENTS}")
	string(REPLACE ";" "\\;" args "${args}")
	add_custom_command(
		${command}
		COMMAND ${CMAKE_COMMAND}
			-DGENERATE_COMMAND_GENERATE=ON
			-DGENERATE_COMMAND_GENERATE_ARGV="${args}"
			-P ${PROJECT_SOURCE_DIR}/cmake/generatecommand.cmake
	)
endfunction()

function(GenerateCommand_Internal)
	execute_process(${ARGN} OUTPUT_QUIET)
endfunction()

if (GENERATE_COMMAND_GENERATE)
	GenerateCommand_Internal(${GENERATE_COMMAND_GENERATE_ARGV})
endif()

