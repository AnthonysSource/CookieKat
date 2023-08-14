cmake_minimum_required(VERSION 3.23)

file(GLOB_RECURSE SRC_FILES
	"*.cpp"
	"*.h"
)

set(TARGET ResourceCompiler)

# ------------------------------------------------------------------------------

add_library(${TARGET} STATIC ${SRC_FILES})

target_include_directories(${TARGET}
PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(${TARGET}
PUBLIC
	CookieKat_Engine
)

set_target_properties(${TARGET} PROPERTIES 
	LINKER_LANGUAGE CXX
	FOLDER CookieKat/Tools
)

# ------------------------------------------------------------------------------

install(TARGETS ${TARGET}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)