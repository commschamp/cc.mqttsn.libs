add_subdirectory (bare_metal_app)

if (NOT CXXTEST_FOUND)
    return ()
endif ()

set (COMPONENT_NAME "cc.mqttsn.client")
set (COMMON_TEST_CLIENT_LIB "CommonTestClient")

#################################################################

function (lib_common_test_client)
    set (src
        "CommonTestClient.cpp"
        "DataProcessor.cpp"
    )
    add_library (${COMMON_TEST_CLIENT_LIB} STATIC ${src})
    add_dependencies(${COMMON_TEST_CLIENT_LIB} ${DEFAULT_CLIENT_LIB_TGT})
    target_link_libraries(${COMMON_TEST_CLIENT_LIB} PUBLIC cc::comms)
    
    if (TARGET cc::mqttsn)
        target_link_libraries(${COMMON_TEST_CLIENT_LIB} PUBLIC cc::mqttsn)    
    endif ()
    
    if (NOT "${CC_MQTTSN_EXTERNAL_TGT_INCLUDE}" STREQUAL "")
        target_include_directories(${COMMON_TEST_CLIENT_LIB} PUBLIC ${CC_MQTTSN_EXTERNAL_TGT_INCLUDE})
    endif ()
    
    if (TARGET ${CC_EXTERNAL_TGT})
        add_dependencies(${COMMON_TEST_CLIENT_LIB} ${CC_EXTERNAL_TGT})
    endif ()
    
    if (TARGET ${CC_MQTTSN_EXTERNAL_TGT})
        add_dependencies(${COMMON_TEST_CLIENT_LIB} ${CC_MQTTSN_EXTERNAL_TGT})
    endif ()
endfunction ()

#################################################################

function (test_func test_suite_name)
    set (tests "${CMAKE_CURRENT_SOURCE_DIR}/${test_suite_name}.th")

    set (name "${COMPONENT_NAME}.${test_suite_name}Test")

    set (runner "${test_suite_name}TestRunner.cpp")
    
    CXXTEST_ADD_TEST (${name} ${runner} ${tests} ${extra_sources})
    target_link_libraries(${name} ${COMMON_TEST_CLIENT_LIB} ${DEFAULT_CLIENT_LIB_TGT})
    
    if (VALGRIND_EXECUTABLE)
        add_test(NAME "${name}_valgrind"
             COMMAND ${CMAKE_COMMAND} 
                -DTEST_EXECUTABLE=$<TARGET_FILE:${name}>
                -DVALGRIND_EXECUTABLE=${VALGRIND_EXECUTABLE}
                -P ${PROJECT_SOURCE_DIR}/cmake/ScriptRunUnitTestWithValgrind.cmake) 
    endif ()
    
endfunction ()

#################################################################

function (test_client_basic)
    test_func ("ClientBasic")
endfunction ()

#################################################################

include_directories (
    "${CXXTEST_INCLUDE_DIR}"
    "${CMAKE_BINARY_DIR}/client/src/basic"
)

if (CMAKE_COMPILER_IS_GNUCC)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-old-style-cast -Wno-shadow")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "7.9")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-ignored-qualifiers")
    endif ()    
endif ()

lib_common_test_client()
test_client_basic()
