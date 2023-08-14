cmake_minimum_required(VERSION 3.23)

# Source & Header files
# ------------------------------------------------------------------------------

set(TARGET CookieKat_App_Win32)

file(GLOB_RECURSE SRC_FILES
	"*.cpp"
	"*.h"
)

# Target Definition
# ------------------------------------------------------------------------------
add_executable(${TARGET} ${SRC_FILES})

target_link_libraries(${TARGET}
PUBLIC
	CookieKat_Engine
	CookieKat_Runtime_Game
)

set_target_properties(${TARGET} PROPERTIES 
	LINKER_LANGUAGE CXX
	FOLDER CookieKat/Applications
	PROJECT_LABEL App_Win32
)

set_property(TARGET ${TARGET} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/")
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT CookieKat_App_Win32)

install(TARGETS ${TARGET}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)