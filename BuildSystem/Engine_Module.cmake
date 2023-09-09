# ------------------------------------------------------------------------------

# Custom target that contains scripts with shared functions 
# used to easily define Core Engine Modules

add_custom_target(CMake_CKE_RN_Core_Shared_Config SOURCES 
	"${CMAKE_CURRENT_SOURCE_DIR}/BuildSystem/Engine_Module.cmake"
)

set_target_properties(CMake_CKE_RN_Core_Shared_Config PROPERTIES 
	FOLDER CookieKat/00_BuildSystem
	PROJECT_LABEL BuildConfig
)

# Generic Runtime Module Definition
# ------------------------------------------------------------------------------

# Function to define a generic runtime engine module project
function(CK_Generic_Module
	TARGET
	SHORT_NAME
	FOLDER_NAME
	HEADER_BASE_DIR
	PUBLIC_LIB_DEPS)
	
	project(${TARGET})

	# We are transitioning to Private/Public folders for cpp/h files but we maintain
	# compatibility with the previous src/include folder structure
	file(GLOB_RECURSE SRC_FILES
		"src/*.cpp"
		"src/*.h"
		"Private/*.cpp"
		"Private/*.h"
	)

	file(GLOB_RECURSE HDR_FILES
		"include/*.h"
		"include/*.cpp"
		"Public/*.h"
		"Public/*.cpp"
	)

	file(GLOB_RECURSE TEST_FILES
		"tests/*.cpp"
	)

	# Target Config
	# ------------------------------------------------------------------------------
	
	add_library(${TARGET} STATIC)
	
	target_sources(${TARGET}
	PRIVATE
		${SRC_FILES} 
		${HDR_FILES}
	)

	target_include_directories(${TARGET}
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/include/"
		"${CMAKE_CURRENT_SOURCE_DIR}/Public/"
	PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/include/${HEADER_BASE_DIR}"
		"${CMAKE_CURRENT_SOURCE_DIR}/Public/${HEADER_BASE_DIR}"
	)
	
	target_link_libraries(${TARGET}
	PUBLIC
		${PUBLIC_LIB_DEPS}
	)
	
	set_target_properties(${TARGET} PROPERTIES 
		LINKER_LANGUAGE CXX
		FOLDER ${FOLDER_NAME}
		PROJECT_LABEL "${SHORT_NAME}"
	)

	target_compile_definitions(${TARGET}
	PUBLIC
		CKE_BUILDSYSTEM_ASSERTS_ENABLE
	)

	target_compile_options(${TARGET}
	PUBLIC
		/arch:AVX2
		$<$<CONFIG:Release>:/O2>  # Maximize Speed
		$<$<CONFIG:Release>:/Ob3> # Inline Function Expansion
		$<$<CONFIG:Release>:/GL>
	)

	target_link_options(${TARGET}
	PUBLIC
		$<$<CONFIG:Release>:/LTCG> # Link-time code generation
	)
	
	# VS Filters
	# ------------------------------------------------------------------------------

	source_group("00_BuildSystem" FILES "CMakeLists.txt")
	
	list(GET HDR_FILES 0 folderStructureCheck)
	string(FIND "${folderStructureCheck}" "include" substringIndex)
	if(substringIndex GREATER_EQUAL 0)
		source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include/${HEADER_BASE_DIR}" PREFIX "Public" FILES ${HDR_FILES})
		source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src/" PREFIX "Private" FILES ${SRC_FILES})
	else()
		source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Public/${HEADER_BASE_DIR}" PREFIX "Public" FILES ${HDR_FILES})
		source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Private/" PREFIX "Private" FILES ${SRC_FILES})
	endif()

	source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/tests/" PREFIX "Tests" FILES ${TEST_FILES})
	
	# Install
	# ------------------------------------------------------------------------------
	
	install(TARGETS ${TARGET}
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	)

endfunction()

# Layer Module Definitions
# ------------------------------------------------------------------------------

function(CK_Core_Module
	SHORT_NAME
	PUBLIC_LIB_DEPS)

	CK_Generic_Module(
		"CookieKat_Runtime_Core_${SHORT_NAME}"
		${SHORT_NAME}
		CookieKat/01_Core
		"CookieKat/Core/${SHORT_NAME}/"
		"${PUBLIC_LIB_DEPS}")

endfunction()

function(CK_Systems_Module
	SHORT_NAME
	PUBLIC_LIB_DEPS)

	CK_Generic_Module(
		"CookieKat_Runtime_Systems_${SHORT_NAME}"
		${SHORT_NAME}
		CookieKat/02_Systems
		"CookieKat/Systems/${SHORT_NAME}/"
		"${PUBLIC_LIB_DEPS}")

endfunction()

function(CK_Engine_Module
	SHORT_NAME
	PUBLIC_LIB_DEPS)

	CK_Generic_Module(
		"CookieKat_Runtime_Engine_${SHORT_NAME}"
		${SHORT_NAME}
		CookieKat/03_Engine
		"CookieKat/Engine/${SHORT_NAME}/"
		"${PUBLIC_LIB_DEPS}")

endfunction()

# Testing Module Definitions
# ------------------------------------------------------------------------------

function(CK_Module_Tests
	ModuleLayer
	ModuleShortName
	ModuleTarget)

	# ------------------------------------------------------------------------------

	project("${ModuleTarget}_Tests")

	set(TestTarget "${ModuleTarget}_Tests")
	
	file(GLOB_RECURSE TEST_FILES
		"tests/*.cpp"
	)
	
	# ------------------------------------------------------------------------------

	enable_testing()
	# find_package(GTest REQUIRED)
	include(GoogleTest)

	add_executable(${TestTarget}
		${TEST_FILES}
	)
	
	target_link_libraries(${TestTarget}
	PRIVATE	
	    GTest::gtest_main
	    ${ModuleTarget}
	)

	set_target_properties(${TestTarget} PROPERTIES 
		LINKER_LANGUAGE CXX
		FOLDER "CookieKat/Tests/${ModuleLayer}"
		PROJECT_LABEL "Tests_${ModuleShortName}"
		WINDOWS_EXPORT_ALL_SYMBOLS YES
		VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/"
	)

	# ------------------------------------------------------------------------------

	install(TARGETS ${TestTarget}
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	)

	add_custom_command(
            TARGET ${TestTarget} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:${TestTarget}> $<TARGET_FILE_DIR:${TestTarget}>
            COMMAND_EXPAND_LISTS
    )

	gtest_discover_tests(${TestTarget})

endfunction()


# Shortcuts to define a test target for a layer specific module
# ------------------------------------------------------------------------------

function(CK_Core_Module_Tests
	SHORT_NAME)

	CK_Module_Tests(
		"01_Core"
		${SHORT_NAME}
		"CookieKat_Runtime_Core_${SHORT_NAME}")

endfunction()

function(CK_Systems_Module_Tests
	SHORT_NAME)

	CK_Module_Tests(
		"02_Systems"
		${SHORT_NAME}
		"CookieKat_Runtime_Systems_${SHORT_NAME}")

endfunction()

function(CK_Engine_Module_Tests
	SHORT_NAME)

	CK_Module_Tests(
		"03_Engine"
		${SHORT_NAME}
		"CookieKat_Runtime_Engine_${SHORT_NAME}")

endfunction()