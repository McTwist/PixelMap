
set(PIXELMAP_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set(PIXELMAP_HEADER_ALPHA
	"${PIXELMAP_INCLUDE_DIR}/alpha/limits.hpp"
	"${PIXELMAP_INCLUDE_DIR}/alpha/v.hpp"
	"${PIXELMAP_INCLUDE_DIR}/alpha/worker.hpp"
	)
set(PIXELMAP_HEADER_ANVIL
	"${PIXELMAP_INCLUDE_DIR}/anvil/factory.hpp"
	"${PIXELMAP_INCLUDE_DIR}/anvil/level.hpp"
	"${PIXELMAP_INCLUDE_DIR}/anvil/limits.hpp"
	"${PIXELMAP_INCLUDE_DIR}/anvil/v.hpp"
	"${PIXELMAP_INCLUDE_DIR}/anvil/v3.hpp"
	"${PIXELMAP_INCLUDE_DIR}/anvil/v13.hpp"
	"${PIXELMAP_INCLUDE_DIR}/anvil/v16.hpp"
	"${PIXELMAP_INCLUDE_DIR}/anvil/v18.hpp"
	"${PIXELMAP_INCLUDE_DIR}/anvil/version.hpp"
	"${PIXELMAP_INCLUDE_DIR}/anvil/worker.hpp"
	)
set(PIXELMAP_HEADER_BEDROCK
	"${PIXELMAP_INCLUDE_DIR}/bedrock/factory.hpp"
	"${PIXELMAP_INCLUDE_DIR}/bedrock/level.hpp"
	"${PIXELMAP_INCLUDE_DIR}/bedrock/limits.hpp"
	"${PIXELMAP_INCLUDE_DIR}/bedrock/parse.hpp"
	"${PIXELMAP_INCLUDE_DIR}/bedrock/v.hpp"
	"${PIXELMAP_INCLUDE_DIR}/bedrock/worker.hpp"
	"${PIXELMAP_INCLUDE_DIR}/bedrock/world.hpp"
	)
set(PIXELMAP_HEADER_BETA
	"${PIXELMAP_INCLUDE_DIR}/beta/worker.hpp"
	)
set(PIXELMAP_HEADER_FORMAT
	"${PIXELMAP_INCLUDE_DIR}/format/alpha.hpp"
	"${PIXELMAP_INCLUDE_DIR}/format/leveldb.hpp"
	"${PIXELMAP_INCLUDE_DIR}/format/nbt.hpp"
	"${PIXELMAP_INCLUDE_DIR}/format/region.hpp"
	"${PIXELMAP_INCLUDE_DIR}/format/varint.hpp"
	)
set(PIXELMAP_HEADER_MODULE
	"${PIXELMAP_INCLUDE_DIR}/module/library.hpp"
	"${PIXELMAP_INCLUDE_DIR}/module/module.hpp"
	)
set(PIXELMAP_HEADER_RENDER
	"${PIXELMAP_INCLUDE_DIR}/render/blockpass.hpp"
	"${PIXELMAP_INCLUDE_DIR}/render/blockpassbuilder.hpp"
	"${PIXELMAP_INCLUDE_DIR}/render/color.hpp"
	"${PIXELMAP_INCLUDE_DIR}/render/image.hpp"
	"${PIXELMAP_INCLUDE_DIR}/render/passbuilder.hpp"
	"${PIXELMAP_INCLUDE_DIR}/render/render.hpp"
	"${PIXELMAP_INCLUDE_DIR}/render/renderpass.hpp"
	"${PIXELMAP_INCLUDE_DIR}/render/renderpassdeclare.hpp"
	"${PIXELMAP_INCLUDE_DIR}/render/renderpassdefine.hpp"
	"${PIXELMAP_INCLUDE_DIR}/render/utility.hpp"
	)
set(PIXELMAP_HEADER_UTIL
	"${PIXELMAP_INCLUDE_DIR}/util/compression.hpp"
	"${PIXELMAP_INCLUDE_DIR}/util/endianess.hpp"
	"${PIXELMAP_INCLUDE_DIR}/util/nibble.hpp"
	"${PIXELMAP_INCLUDE_DIR}/util/palette.hpp"
	)
set(PIXELMAP_HEADER
	"${PIXELMAP_HEADER_ALPHA}"
	"${PIXELMAP_HEADER_ANVIL}"
	"${PIXELMAP_HEADER_BEDROCK}"
	"${PIXELMAP_HEADER_BETA}"
	"${PIXELMAP_HEADER_FORMAT}"
	"${PIXELMAP_HEADER_MODULE}"
	"${PIXELMAP_HEADER_RENDER}"
	"${PIXELMAP_HEADER_UTIL}"
	"${PIXELMAP_INCLUDE_DIR}/any.hpp"
	"${PIXELMAP_INCLUDE_DIR}/blockcolor.hpp"
	"${PIXELMAP_INCLUDE_DIR}/chunk.hpp"
	"${PIXELMAP_INCLUDE_DIR}/delayedaccumulator.hpp"
	"${PIXELMAP_INCLUDE_DIR}/eventhandler.hpp"
	"${PIXELMAP_INCLUDE_DIR}/libraryoptions.hpp"
	"${PIXELMAP_INCLUDE_DIR}/lightsource.hpp"
	"${PIXELMAP_INCLUDE_DIR}/limits.hpp"
	"${PIXELMAP_INCLUDE_DIR}/log.hpp"
	"${PIXELMAP_INCLUDE_DIR}/lonely.hpp"
	"${PIXELMAP_INCLUDE_DIR}/minecraft.hpp"
	"${PIXELMAP_INCLUDE_DIR}/options.hpp"
	"${PIXELMAP_INCLUDE_DIR}/performance.hpp"
	"${PIXELMAP_INCLUDE_DIR}/pixelmap.hpp"
	"${PIXELMAP_INCLUDE_DIR}/platform.hpp"
	"${PIXELMAP_INCLUDE_DIR}/semaphore.hpp"
	"${PIXELMAP_INCLUDE_DIR}/shared_counter.hpp"
	"${PIXELMAP_INCLUDE_DIR}/shared_value.hpp"
	"${PIXELMAP_INCLUDE_DIR}/sharedfile.hpp"
	"${PIXELMAP_INCLUDE_DIR}/string.hpp"
	"${PIXELMAP_INCLUDE_DIR}/threadpool.hpp"
	"${PIXELMAP_INCLUDE_DIR}/threadworker.hpp"
	"${PIXELMAP_INCLUDE_DIR}/timer.hpp"
	"${PIXELMAP_INCLUDE_DIR}/vectorview.hpp"
	"${PIXELMAP_INCLUDE_DIR}/worker.hpp"
	)

set(PIXELMAP_SRC_ALPHA
	"alpha/v.cpp"
	"alpha/worker.cpp"
	)
set(PIXELMAP_SRC_ANVIL
	"anvil/factory.cpp"
	"anvil/level.cpp"
	"anvil/v.cpp"
	"anvil/v3.cpp"
	"anvil/v13.cpp"
	"anvil/v16.cpp"
	"anvil/v18.cpp"
	"anvil/worker.cpp"
	)
set(PIXELMAP_SRC_BEDROCK
	"bedrock/factory.cpp"
	"bedrock/level.cpp"
	"bedrock/parse.cpp"
	"bedrock/v.cpp"
	"bedrock/worker.cpp"
	"bedrock/world.cpp"
	)
set(PIXELMAP_SRC_BETA
	"beta/worker.cpp"
	)
set(PIXELMAP_SRC_FORMAT
	"format/alpha.cpp"
	"format/leveldb.cpp"
	"format/nbt.cpp"
	"format/region.cpp"
	"format/snbt.cpp"
	)
set(PIXELMAP_SRC_MODULE
	"module/library.cpp"
	)
set(PIXELMAP_SRC_RENDER
	"render/blockpass.cpp"
	"render/blockpassbuilder.cpp"
	"render/chunkpass.cpp"
	"render/color.cpp"
	"render/image.cpp"
	"render/regionpass.cpp"
	"render/renderpass.cpp"
	"render/render.cpp"
	"render/utility.cpp"
	"render/worldpass.cpp"
	)
set(PIXELMAP_SRC_UTIL
	"util/compression.cpp"
	"util/palette.cpp"
	)
set(PIXELMAP_SRC
	"${PIXELMAP_SRC_ALPHA}"
	"${PIXELMAP_SRC_ANVIL}"
	"${PIXELMAP_SRC_BEDROCK}"
	"${PIXELMAP_SRC_BETA}"
	"${PIXELMAP_SRC_FORMAT}"
	"${PIXELMAP_SRC_MODULE}"
	"${PIXELMAP_SRC_RENDER}"
	"${PIXELMAP_SRC_UTIL}"
	"blockcolor.cpp"
	"chunk.cpp"
	"lightsource.cpp"
	"log.cpp"
	"lonely.cpp"
	"minecraft.cpp"
	"pixelmap.cpp"
	"platform.cpp"
	"sharedfile.cpp"
	"threadpool.cpp"
	"threadworker.cpp"
	"worker.cpp"
	)

# Add source groups
source_group(alpha FILES ${PIXELMAP_HEADER_ALPHA} ${PIXELMAP_SRC_ALPHA})
source_group(anvil FILES ${PIXELMAP_HEADER_ANVIL} ${PIXELMAP_SRC_ANVIL})
source_group(bedrock FILES ${PIXELMAP_HEADER_BEDROCK} ${PIXELMAP_SRC_BEDROCK})
source_group(beta FILES ${PIXELMAP_HEADER_BETA} ${PIXELMAP_SRC_BETA})
source_group(format FILES ${PIXELMAP_HEADER_FORMAT} ${PIXELMAP_SRC_FORMAT})
source_group(module FILES ${PIXELMAP_HEADER_MODULE} ${PIXELMAP_SRC_MODULE})
source_group(render FILES ${PIXELMAP_HEADER_RENDER} ${PIXELMAP_SRC_RENDER})
source_group(util FILES ${PIXELMAP_HEADER_UTIL} ${PIXELMAP_SRC_UTIL})

EmbedFile("resource/blockcolor.conf" BLOCKCOLOR_CPP BLOCKCOLOR_HPP)
EmbedFile("resource/lightsource.conf" LIGHTSOURCE_CPP LIGHTSOURCE_HPP)

# Add include directories from libraries
include_directories(
	${DEFLATE_INCLUDE_DIRS}
	${ZLIB_INCLUDE_DIRS}
	${LZ4_INCLUDE_DIRS}
	${PNG_INCLUDE_DIRS}
	${GLM_INCLUDE_DIRS}
	${FMT_INCLUDE_DIRS}
	${SPDLOG_INCLUDE_DIRS})

# Create the static library
add_library(pixelmap STATIC ${PIXELMAP_HEADER} ${PIXELMAP_SRC})

target_sources(pixelmap PRIVATE ${BLOCKCOLOR_HPP} ${BLOCKCOLOR_CPP})
target_sources(pixelmap PRIVATE ${LIGHTSOURCE_HPP} ${LIGHTSOURCE_CPP})

if(TARGET pixelmapweb)
	message(STATUS "Enable WebView")
	target_compile_definitions(pixelmap PUBLIC ENABLE_WEBVIEW)
	add_dependencies(pixelmap pixelmapweb)
	target_include_directories(pixelmap PRIVATE
		$<BUILD_INTERFACE:${WEBVIEW_INCLUDE_DIRS}>)
	target_sources(pixelmap PRIVATE
		"${PIXELMAP_INCLUDE_DIR}/util/webview.hpp"
		"util/webview.cpp"
		)
endif()

target_include_directories(pixelmap PUBLIC
	$<BUILD_INTERFACE:${PIXELMAP_INCLUDE_DIR}>
	$<BUILD_INTERFACE:${EMBED_INCLUDE_DIR}>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)

set_target_properties(pixelmap PROPERTIES DEFINE_SYMBOL PIXELMAP_DLL)

# Warning levels
target_compile_options(pixelmap PUBLIC
	$<$<CXX_COMPILER_ID:MSVC>:/W4>
	$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Wno-long-long>
)

if(PIXELMAP_USE_LIBDEFLATE)
	target_compile_definitions(pixelmap PRIVATE USE_LIBDEFLATE)
endif()
if(PIXELMAP_ENABLE_AFFINITY)
	message(STATUS "Enable thread affinity")
	target_compile_definitions(pixelmap PRIVATE ENABLE_AFFINITY)
endif()
if(PIXELMAP_PROFILE)
	message(STATUS "Enable profiling")
	target_compile_definitions(pixelmap PUBLIC PERF_DEBUG)
endif()

# Link everything together
target_link_libraries(pixelmap
	PRIVATE ${WEBVIEW_LIBRARY} ${ZLIB_LIBRARIES} ${DEFLATE_LIBRARIES} ${LZ4_LIBRARIES} ${PNG_LIBRARIES}
	PUBLIC ${FMT_LIBRARIES} ${SPDLOG_LIBRARIES}
	PRIVATE ${CMAKE_DL_LIBS} Threads::Threads)
