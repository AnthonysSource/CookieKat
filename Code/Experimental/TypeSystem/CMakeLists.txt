cmake_minimum_required(VERSION 3.23)

# ------------------------------------------------------------------------------

file(GLOB_RECURSE SRC_FILES
	"*.cpp"
	"*.h"
)

set(TARGET TypeSystem)

# ------------------------------------------------------------------------------

add_executable(${TARGET} ${SRC_FILES})

target_link_libraries(${TARGET}
PUBLIC
	CookieKat_Engine
)

set_target_properties(${TARGET} PROPERTIES 
	LINKER_LANGUAGE CXX
	FOLDER CookieKat/Experimental
)

# ------------------------------------------------------------------------------

install(TARGETS ${TARGET}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)