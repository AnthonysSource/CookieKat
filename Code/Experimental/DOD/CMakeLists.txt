

set(TARGET CPUCacheTests)

add_executable(${TARGET})

file(GLOB_RECURSE SRC_FILES
	"*.cpp"
	"*.h"
)

target_sources(${TARGET} 
PUBLIC
	${SRC_FILES}
)

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
	FOLDER CookieKat/Experimental
)

install(TARGETS ${TARGET}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)