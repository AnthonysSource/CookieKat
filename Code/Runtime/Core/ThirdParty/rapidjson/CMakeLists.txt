
file(GLOB_RECURSE SRC_FILES
	"*.h"
	"*.cpp"
)

set(TR RapidJSON)

add_library(${TR} STATIC)
target_sources(${TR}
PRIVATE
	${SRC_FILES}
)

target_include_directories(${TR}
PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/include"
)