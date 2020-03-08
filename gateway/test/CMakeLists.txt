if (NOT CXXTEST_FOUND)
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
    target_link_libraries(${COMMON_TEST_SESSION_LIB} PRIVATE cc::comms)
    add_dependencies (${COMMON_TEST_SESSION_LIB} ${MQTTSN_GATEWAY_LIB_NAME})
    
    if (TARGET cc::mqtt311)
        target_link_libraries(${COMMON_TEST_SESSION_LIB} PRIVATE cc::mqtt311)    
    endif ()
    
    if (TARGET cc::mqttsn)
        target_link_libraries(${COMMON_TEST_SESSION_LIB} PRIVATE cc::mqttsn)    
    endif ()
    
    if (NOT "${CC_MQTT_EXTERNAL_TGT_INCLUDE}" STREQUAL "")
        target_include_directories(${COMMON_TEST_SESSION_LIB} PRIVATE ${CC_MQTT_EXTERNAL_TGT_INCLUDE})
    endif ()
    
    if (NOT "${CC_MQTTSN_EXTERNAL_TGT_INCLUDE}" STREQUAL "")
        target_include_directories(${COMMON_TEST_SESSION_LIB} PRIVATE ${CC_MQTTSN_EXTERNAL_TGT_INCLUDE})
    endif ()
    
    if (TARGET ${CC_EXTERNAL_TGT})
        add_dependencies(${COMMON_TEST_SESSION_LIB} ${CC_EXTERNAL_TGT})
    endif ()
    
    if (TARGET ${CC_MQTT_EXTERNAL_TGT})
        add_dependencies(${COMMON_TEST_SESSION_LIB} ${CC_MQTT_EXTERNAL_TGT})
    endif ()

    if (TARGET ${CC_MQTTSN_EXTERNAL_TGT})
        add_dependencies(${COMMON_TEST_SESSION_LIB} ${CC_MQTTSN_EXTERNAL_TGT})
    endif ()
    
endfunction ()

#################################################################

function (test_func test_suite_name)
    set (tests "${CMAKE_CURRENT_SOURCE_DIR}/${test_suite_name}.th")

    set (name "${COMPONENT_NAME}.${test_suite_name}Test")

    set (runner "${test_suite_name}TestRunner.cpp")
    
    CXXTEST_ADD_TEST (${name} ${runner} ${tests} ${extra_sources})
    target_link_libraries(${name} ${COMMON_TEST_SESSION_LIB} ${MQTTSN_GATEWAY_LIB_NAME} cc::comms)
    
    if (TARGET cc::mqtt311)
        target_link_libraries(${name} PRIVATE cc::mqtt311)    
    endif ()
    
    if (TARGET cc::mqttsn)
        target_link_libraries(${name} PRIVATE cc::mqtts )    
    endif ()
    
    if (NOT "${CC_MQTT_EXTERNAL_TGT_INCLUDE}" STREQUAL "")
        target_include_directories(${name} PRIVATE ${CC_MQTT_EXTERNAL_TGT_INCLUDE})
    endif ()
    
    if (NOT "${CC_MQTTSN_EXTERNAL_TGT_INCLUDE}" STREQUAL "")
        target_include_directories(${name} PRIVATE ${CC_MQTTSN_EXTERNAL_TGT_INCLUDE})
    endif ()
    
    if (TARGET ${CC_EXTERNAL_TGT})
        add_dependencies(${name} ${CC_EXTERNAL_TGT})
    endif ()
    
    if (TARGET ${CC_MQTT_EXTERNAL_TGT})
        add_dependencies(${name} ${CC_MQTT_EXTERNAL_TGT})
    endif ()
    
    if (TARGET ${CC_MQTTSN_EXTERNAL_TGT})
        add_dependencies(${name} ${CC_MQTTSN_EXTERNAL_TGT})
    endif ()    
    
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
