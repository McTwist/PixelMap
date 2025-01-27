#[===[
Custom ExternalData script
Used to apply headers when downloading. Otherwise
basically copied from _ExternalData_download_file.
This is totally unnecessary for this type of project,
but at least it is consistent
]===]

set(retry 3)
while(retry)
	math(EXPR retry "${retry} - 1")
	if(ExternalData_TIMEOUT_INACTIVITY)
		set(inactivity_timeout INACTIVITY_TIMEOUT ${ExternalData_TIMEOUT_INACTIVITY})
	elseif(NOT "${ExternalData_TIMEOUT_INACTIVITY}" EQUAL 0)
		set(inactivity_timeout INACTIVITY_TIMEOUT 60)
	else()
		set(inactivity_timeout "")
	endif()
	if(ExternalData_TIMEOUT_ABSOLUTE)
		set(absolute_timeout TIMEOUT ${ExternalData_TIMEOUT_ABSOLUTE})
	elseif(NOT "${ExternalData_TIMEOUT_ABSOLUTE}" EQUAL 0)
		set(absolute_timeout TIMEOUT 60)
	else()
		set(absolute_timeout "")
	endif()
	set(show_progress_args)
	if(ExternalData_SHOW_PROGRESS)
		list(APPEND show_progress_args SHOW_PROGRESS)
	endif()
	file(DOWNLOAD "${ExternalData_CUSTOM_LOCATION}"
		"${ExternalData_CUSTOM_FILE}"
		HTTPHEADER "Accept-Encoding: *"
		STATUS status LOG log ${inactivity_timeout} ${absolute_timeout} ${show_progress_args})
	list(GET status 0 err)
	list(GET status 1 msg)
	if(err)
		if("${msg}" MATCHES "partial|timeout|temporarily" AND
			"${log}" MATCHES "error: 503")
			set(msg "temporarily unavailable")
		endif()
	elseif("${log}" MATCHES "\nHTTP[^\n]* 503")
		set(err TRUE)
		set(msg "temporarily unavailable")
	endif()
	if(NOT err OR NOT "${msg}" MATCHES "partial|timeout|temporarily")
		break()
	else()
		message(STATUS "[download terminated: ${msg}, retries left: ${retry}]")
	endif()
endwhile()
if(err)
	set(ExternalData_CUSTOM_ERROR "${msg}" PARENT_SCOPE)
endif()
