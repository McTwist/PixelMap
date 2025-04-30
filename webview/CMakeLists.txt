cmake_minimum_required(VERSION 3.24)

project(PixelMapWeb LANGUAGES CXX)

set(WEBVIEW_SOURCE
	"index.html"
	"main.js"
	"style.css"
	"vite.config.js"
	"package.json"
)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Optionally look for npm, and do not build if not found
find_program(NPM npm)

if(NPM)
	set(dist_dir "${PROJECT_SOURCE_DIR}/dist")
	set(dist_files
		"${dist_dir}/index.html"
		"${dist_dir}/index.js"
		"${dist_dir}/index.css")
	set(out_archive "${PROJECT_BINARY_DIR}/webview.fgz")
	include(generatecommand)
	include(embedfile)

	execute_process(
		COMMAND ${NPM} install
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	)

	# Build source with npm, creating an archive...
	GenerateCommand(
		OUTPUT ${out_archive}
		DEPENDS ${WEBVIEW_SOURCE}
		COMMENT "Building pixelmapweb"
		BYPRODUCTS ${dist_files}
		TERMINATE
		COMMAND ${CMAKE_COMMAND} -E env "ARCHIVE_OUT=${out_archive}" ${NPM} run build
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	)
	# ... which is embeded in source files...
	EmbedFile("${out_archive}" WEBVIEW_CPP WEBVIEW_HPP
		IDENTIFIER "webview.fgz"
	)
	# ... and added to a library.
	add_library(pixelmapweb STATIC ${WEBVIEW_HPP} ${WEBVIEW_CPP})

	set(WEBVIEW_INCLUDE_DIRS ${EMBED_INCLUDE_DIR} PARENT_SCOPE)
	set(WEBVIEW_LIBRARY pixelmapweb PARENT_SCOPE)
	set(OPENLAYERS_LICENSE "${PROJECT_SOURCE_DIR}/node_modules/ol/LICENSE.md" PARENT_SCOPE)
else()
	message(WARNING "Missing NPM, skipping WebView")
endif()
