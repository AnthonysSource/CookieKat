# ==============================================================================
#  CookieKat Build System
# ==============================================================================
cmake_minimum_required(VERSION 3.23)

# Project declaration
# ------------------------------------------------------------------------------

project("CookieKat"
	VERSION 1.0.0
	LANGUAGES C CXX
)

# NOTE: THIS IS USER DEFINED
# ------------------------------------------------------------------------------

set(Vulkan_INCLUDE_DIR "D:/Programs/VulkanSDK/1.3.236.0/Include")
set(Vulkan_LIBRARY "D:/Programs/VulkanSDK/1.3.236.0/Lib/vulkan-1.lib")
find_package(Vulkan REQUIRED)

# Global Properties
# ------------------------------------------------------------------------------

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Visual Studio Folders & Filters Setup
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "00_CMake")
set(CMAKE_FOLDER External) # Default VS folder for targets
# Move all of the .exe to the same install folder
set(CMAKE_VS_INCLUDE_INSTALL_TO_DEFAULT_BUILD ON) 

# Build Auxiliary Functionality
# ------------------------------------------------------------------------------

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/BuildSystem")
include("BuildSystem/Engine_Module.cmake")

# Targets
# ------------------------------------------------------------------------------

# Runtime
add_subdirectory("Code/Runtime")

# Tools
add_subdirectory("Code/Tools/ResourceCompiler")
add_subdirectory("Code/Tools/ResourceCompilerCLI")

# In-Development Experimental Systems
add_subdirectory("Code/Experimental/Serialization")
add_subdirectory("Code/Experimental/TypeSystem")
add_subdirectory("Code/Experimental/DOD")
add_subdirectory("Code/Experimental/ECS")
add_subdirectory("Code/Experimental/Vulkan")
add_subdirectory("Code/Experimental/Vulkan_2")
add_subdirectory("Code/Experimental/Vulkan_3")
add_subdirectory("Code/Experimental/SmallTests")
add_subdirectory("Code/Experimental/RenderAPI_Sandbox")