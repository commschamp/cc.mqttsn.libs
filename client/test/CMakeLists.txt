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
endfunction ()

#################################################################

function (test_func test_suite_name)
    set (tests "${CMAKE_CURRENT_SOURCE_DIR}/${test_suite_name}.th")

    set (name "${COMPONENT_NAME}.${test_suite_name}Test")

    set (runner "${test_suite_name}TestRunner.cpp")
    
    CXXTEST_ADD_TEST (${name} ${runner} ${tests} ${extra_sources})
    target_link_libraries(${name} ${COMMON_TEST_CLIENT_LIB} ${DEFAULT_CLIENT_LIB_TGT})
    
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

lib_common_test_client()
test_client_basic()
