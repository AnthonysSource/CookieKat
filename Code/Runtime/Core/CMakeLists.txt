cmake_minimum_required(VERSION 3.23)

# Target Definition
# ------------------------------------------------------------------------------

set(TARGET CookieKat_Core)

add_library(${TARGET} STATIC)

# Sources
# ------------------------------------------------------------------------------

set(SRC_FILES "Core.cpp")

target_sources(${TARGET}
PRIVATE
	${SRC_FILES}
)

target_link_libraries(${TARGET}
PUBLIC
	CookieKat_Runtime_Core_Memory
	CookieKat_Runtime_Core_Containers
	CookieKat_Runtime_Core_Platform
	CookieKat_Runtime_Core_Serialization
	CookieKat_Runtime_Core_FileSystem
	CookieKat_Runtime_Core_Random
	CookieKat_Runtime_Core_Math
	CookieKat_Runtime_Core_Time
	CookieKat_Runtime_Core_Profilling
	CookieKat_Runtime_Core_Logging
	CookieKat_Runtime_Core_Threading
	CookieKat_Runtime_Core_Debugging
	"opengl32.lib"
	Glad
	glfw
	imgui
	Stb
	enkiTS
	assimp
	LodePNG
	VulkanMemoryAllocator
)

target_include_directories(${TARGET}
PUBLIC 
	"${CMAKE_CURRENT_SOURCE_DIR}"
)

target_compile_definitions(${TARGET}
PUBLIC
	#CKE_BUILDING_DLL
	GLFW_INCLUDE_NONE
	CKE_GRAPHICS_VULKAN_BACKEND
PRIVATE
	#CKE_BUILD_IMPORT_LIB
)

set_target_properties(${TARGET} PROPERTIES
	LINKER_LANGUAGE CXX
	FOLDER CookieKat/01_Core
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	PROJECT_LABEL 01_Core_Layer
)

target_compile_options(${TARGET}
PUBLIC
	/arch:AVX2
	/Oi
	/Ot
	#/GL
)

# Install
# ------------------------------------------------------------------------------

install(TARGETS ${TARGET}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Third Party
# ------------------------------------------------------------------------------

add_subdirectory("ThirdParty/glad")
add_subdirectory("ThirdParty/glfw")
add_subdirectory("ThirdParty/glm")
add_subdirectory("ThirdParty/imgui")
add_subdirectory("ThirdParty/optick")
add_subdirectory("ThirdParty/stb")
add_subdirectory("ThirdParty/enkiTS")
add_subdirectory("ThirdParty/assimp")
add_subdirectory("ThirdParty/rapidjson")
add_subdirectory("ThirdParty/lodepng")
add_subdirectory("ThirdParty/VulkanMemoryAllocator")

add_subdirectory("ThirdParty/googletest")

# Core Modules
# ------------------------------------------------------------------------------

add_subdirectory("Memory")
add_subdirectory("Containers")
add_subdirectory("Serialization")
add_subdirectory("FileSystem")
add_subdirectory("Random")
add_subdirectory("Platform")
add_subdirectory("Math")
add_subdirectory("Timer")
add_subdirectory("Profilling")
add_subdirectory("Logging")
add_subdirectory("Threading")
add_subdirectory("Debugging")