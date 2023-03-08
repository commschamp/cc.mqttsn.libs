if (NOT BUILD_TESTING)
    return ()
endif ()  

set (COMPONENT_NAME "cc.mqttsn.gateway")
set (COMMON_TEST_SESSION_LIB "CommonTestSession")

#################################################################

function (lib_common_test_session)
    set (src
        "TestMsgHandler.cpp"
    )
    add_library (${COMMON_TEST_SESSION_LIB} STATIC ${src})
    target_link_libraries(${COMMON_TEST_SESSION_LIB} PRIVATE cc::cc_mqtt311 cc::cc_mqttsn cc::comms)
    add_dependencies (${COMMON_TEST_SESSION_LIB} ${MQTTSN_GATEWAY_LIB_NAME})
    
endfunction ()

#################################################################

function (test_func test_suite_name)
    set (tests "${CMAKE_CURRENT_SOURCE_DIR}/${test_suite_name}.th")

    set (name "${COMPONENT_NAME}.${test_suite_name}Test")

    set (valgrand_args)
    if (VALGRIND_EXECUTABLE)
        set (valgrand_args VALGRIND_EXECUTABLE ${VALGRIND_EXECUTABLE})
    endif ()     

    cc_cxxtest_add_test(
        NAME ${name}
        SRC ${tests}
        ${valgrand_args})    

    target_link_libraries(${name} PRIVATE ${COMMON_TEST_SESSION_LIB} ${MQTTSN_GATEWAY_LIB_NAME} cc::cc_mqtt311 cc::cc_mqttsn cc::comms)
endfunction ()

#################################################################

function (test_gateway)
    test_func ("Gateway")
endfunction ()

#################################################################

function (test_session)
    test_func ("Session")
endfunction ()

#################################################################

include_directories (
    "${CXXTEST_INCLUDE_DIR}"
)

lib_common_test_session()
test_gateway()
test_session()
