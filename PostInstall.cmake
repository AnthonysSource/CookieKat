# set(UNIT_TEST CookieKat_Engine_Tests)
# add_custom_command(
#      TARGET ${UNIT_TEST}
#      COMMENT "Run tests"
#      POST_BUILD
#      WORKING_DIRECTORY ${CMAKE_INSTALL_BINDIR}
#      COMMAND ${UNIT_TEST}
# )

set(TestExecName CookieKat_Engine_Tests)
set(WorkDir ${CMAKE_CURRENT_LIST_DIR}/Output/Install/x64-debug/bin)

message(STATUS ${WorkDir})

execute_process(
    COMMAND ${TestExecName}
    WORKING_DIRECTORY ${WorkDir}
    RESULT_VARIABLE TestsResult
    OUTPUT_VARIABLE TestsOutput
)

message(STATUS "${TestsResult} / ${TestsOutput}")
