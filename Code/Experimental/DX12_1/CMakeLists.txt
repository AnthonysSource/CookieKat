cmake_minimum_required(VERSION 3.23)

# ------------------------------------------------------------------------------

file(GLOB_RECURSE SRC_FILES
	"*.cpp"
	"*.h"
)

set(TARGET CookieKat_DX12_V3)

# ------------------------------------------------------------------------------

add_executable(${TARGET} ${SRC_FILES})

find_package(Vulkan REQUIRED)

target_link_libraries(${TARGET}
PUBLIC
	CookieKat_Runtime_Core
)

set_target_properties(${TARGET} PROPERTIES 
	LINKER_LANGUAGE CXX
	FOLDER CookieKat/Experimental
)

set_property(TARGET ${TARGET} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/")

# ------------------------------------------------------------------------------

install(TARGETS ${TARGET}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)