
# Source files
#-------------------------------------------------------------------------
file(GLOB SRC_FILES
    "*.cpp"
    "*.h"
    "backends/imgui_impl_glfw.cpp"
    "backends/imgui_impl_glfw.h"
    "backends/imgui_impl_opengl3.cpp"
    "backends/imgui_impl_opengl3.h"
)

# Creation & Instalation
#-------------------------------------------------------------------------
add_library(imgui STATIC ${SRC_FILES})

target_include_directories(imgui
PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/"
)

target_link_libraries(imgui
PUBLIC
    glfw
)

install(TARGETS imgui
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
