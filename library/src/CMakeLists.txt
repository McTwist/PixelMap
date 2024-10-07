
set(PIXELMAP_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
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
set(PIXELMAP_HEADER_FORMAT
	"${PIXELMAP_INCLUDE_DIR}/format/anvil.hpp"
	"${PIXELMAP_INCLUDE_DIR}/format/leveldb.hpp"
	"${PIXELMAP_INCLUDE_DIR}/format/nbt.hpp"
	"${PIXELMAP_INCLUDE_DIR}/format/varint.hpp"
	)
set(PIXELMAP_HEADER_MODULE
	"${PIXELMAP_INCLUDE_DIR}/module/library.hpp"
	"${PIXELMAP_INCLUDE_DIR}/module/module.hpp"
	)
set(PIXELMAP_HEADER_RENDER
	"${PIXELMAP_INCLUDE_DIR}/render/color.hpp"
	"${PIXELMAP_INCLUDE_DIR}/render/image.hpp"
	"${PIXELMAP_INCLUDE_DIR}/render/passbuilder.hpp"
	"${PIXELMAP_INCLUDE_DIR}/render/render.hpp"
	"${PIXELMAP_INCLUDE_DIR}/render/renderpass.hpp"
	"${PIXELMAP_INCLUDE_DIR}/render/renderpassbuilder.hpp"
	"${PIXELMAP_INCLUDE_DIR}/render/utility.hpp"
	)
set(PIXELMAP_HEADER_UTIL
	"${PIXELMAP_INCLUDE_DIR}/util/compression.hpp"
	"${PIXELMAP_INCLUDE_DIR}/util/endianess.hpp"
	"${PIXELMAP_INCLUDE_DIR}/util/nibble.hpp"
	"${PIXELMAP_INCLUDE_DIR}/util/palette.hpp"
	)
set(PIXELMAP_HEADER
	"${PIXELMAP_HEADER_ANVIL}"
	"${PIXELMAP_HEADER_BEDROCK}"
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
set(PIXELMAP_SRC_FORMAT
	"format/anvil.cpp"
	"format/leveldb.cpp"
	"format/nbt.cpp"
	"format/snbt.cpp"
	)
set(PIXELMAP_SRC_MODULE
	"module/library.cpp"
	)
set(PIXELMAP_SRC_RENDER
	"render/color.cpp"
	"render/image.cpp"
	"render/render.cpp"
	"render/renderpass.cpp"
	"render/renderpassbuilder.cpp"
	"render/utility.cpp"
	)
set(PIXELMAP_SRC_UTIL
	"util/compression.cpp"
	"util/palette.cpp"
	)
set(PIXELMAP_SRC
	"${PIXELMAP_SRC_ANVIL}"
	"${PIXELMAP_SRC_BEDROCK}"
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
source_group(anvil FILES ${PIXELMAP_HEADER_ANVIL} ${PIXELMAP_SRC_ANVIL})
source_group(bedrock FILES ${PIXELMAP_HEADER_BEDROCK} ${PIXELMAP_SRC_BEDROCK})
source_group(format FILES ${PIXELMAP_HEADER_FORMAT} ${PIXELMAP_SRC_FORMAT})
source_group(module FILES ${PIXELMAP_HEADER_MODULE} ${PIXELMAP_SRC_MODULE})
source_group(render FILES ${PIXELMAP_HEADER_RENDER} ${PIXELMAP_SRC_RENDER})
source_group(util FILES ${PIXELMAP_HEADER_UTIL} ${PIXELMAP_SRC_UTIL})

EmbedFile("resource/blockcolor.conf" BLOCKCOLOR_CPP BLOCKCOLOR_HPP)
EmbedFile("resource/lightsource.conf" LIGHTSOURCE_CPP LIGHTSOURCE_HPP)

# Add include directories from libraries
include_directories(${DEFLATE_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS} ${PNG_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS} ${SPDLOG_INCLUDE_DIRS})

# Create the static library
add_library(pixelmap STATIC ${PIXELMAP_HEADER} ${PIXELMAP_SRC})

target_sources(pixelmap PRIVATE ${BLOCKCOLOR_HPP} ${BLOCKCOLOR_CPP})
target_sources(pixelmap PRIVATE ${LIGHTSOURCE_HPP} ${LIGHTSOURCE_CPP})

target_include_directories(pixelmap PUBLIC
	$<BUILD_INTERFACE:${PIXELMAP_INCLUDE_DIR}>
	$<BUILD_INTERFACE:${EMBED_INCLUDE_DIR}>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)

set_target_properties(pixelmap PROPERTIES DEFINE_SYMBOL PIXELMAP_DLL)

# Link everything together
target_link_libraries(pixelmap
	PRIVATE ${ZLIB_LIBRARIES}
	PRIVATE ${DEFLATE_LIBRARIES}
	PRIVATE ${PNG_LIBRARIES}
	PUBLIC ${SPDLOG_LIBRARIES}
	PRIVATE ${CMAKE_DL_LIBS}
	PRIVATE Threads::Threads)
