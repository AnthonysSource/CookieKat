cmake_minimum_required(VERSION 3.23)

# Target Definition
# ------------------------------------------------------------------------------

set(TARGET CookieKat_Systems)

add_library(${TARGET} STATIC)

# Sources
# ------------------------------------------------------------------------------

# We get all the files in the current dir
# except those from the thirdparty folder
file(GLOB_RECURSE SRC_FILES 
	"*.cpp"
	"*.h"
)
list(FILTER SRC_FILES EXCLUDE REGEX "ECS\/.*\.(c|cpp|h|hpp)")
list(FILTER SRC_FILES EXCLUDE REGEX "TaskSystem\/.*\.(c|cpp|h|hpp)")
list(FILTER SRC_FILES EXCLUDE REGEX "EngineSystem\/.*\.(c|cpp|h|hpp)")
list(FILTER SRC_FILES EXCLUDE REGEX "Resources\/.*\.(c|cpp|h|hpp)")
list(FILTER SRC_FILES EXCLUDE REGEX "Input\/.*\.(c|cpp|h|hpp)")
list(FILTER SRC_FILES EXCLUDE REGEX "RenderUtils\/.*\.(c|cpp|h|hpp)")
list(FILTER SRC_FILES EXCLUDE REGEX "RenderAPI\/.*\.(c|cpp|h|hpp)")
list(FILTER SRC_FILES EXCLUDE REGEX "FrameGraph\/.*\.(c|cpp|h|hpp)")

target_sources(${TARGET}
PRIVATE
	${SRC_FILES}
)

target_link_libraries(${TARGET}
PUBLIC
	CookieKat_Core
	CookieKat_Runtime_Systems_TaskSystem
	CookieKat_Runtime_Systems_ECS
	CookieKat_Runtime_Systems_EngineSystem
	CookieKat_Runtime_Systems_Resources
	CookieKat_Runtime_Systems_Input
	CookieKat_Runtime_Systems_RenderAPI
	CookieKat_Runtime_Systems_RenderUtils
	CookieKat_Runtime_Systems_FrameGraph
)

target_include_directories(${TARGET}
PUBLIC 
	"${CMAKE_CURRENT_SOURCE_DIR}"
)

set_target_properties(${TARGET} PROPERTIES
	LINKER_LANGUAGE CXX
	FOLDER CookieKat/02_Systems
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	PROJECT_LABEL 01_Systems_Layer
)

# ------------------------------------------------------------------------------

install(TARGETS ${TARGET}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# ------------------------------------------------------------------------------

add_subdirectory("EngineSystem")

add_subdirectory("ECS")
add_subdirectory("Resources")
add_subdirectory("TaskSystem")
add_subdirectory("RenderAPI")
add_subdirectory("RenderUtils")
add_subdirectory("FrameGraph")
add_subdirectory("Input")