cmake_minimum_required(VERSION 3.23)

# Target Definition
# ------------------------------------------------------------------------------

set(TARGET CookieKat_Engine)

add_library(${TARGET} STATIC)

# Sources
# ------------------------------------------------------------------------------

target_sources(${TARGET}
PRIVATE
	"Engine.cpp"
	"Engine.h"
)

target_link_libraries(${TARGET}
PUBLIC
	CookieKat_Core
	CookieKat_Systems
	CookieKat_Runtime_Engine_Resources
	CookieKat_Runtime_Engine_Render
	CookieKat_Runtime_Engine_Entities
)

target_include_directories(${TARGET}
PUBLIC 
	"${CMAKE_CURRENT_SOURCE_DIR}"
)

set_target_properties(${TARGET} PROPERTIES 
	LINKER_LANGUAGE CXX
	FOLDER CookieKat/03_Engine
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	PROJECT_LABEL 01_Engine_Layer
)

# Install
# ------------------------------------------------------------------------------

install(TARGETS ${TARGET}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# ------------------------------------------------------------------------------

add_subdirectory("Resources")
add_subdirectory("Render")
add_subdirectory("Entities")