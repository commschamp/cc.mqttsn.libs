if (NOT CC_MQTTSN_BUILD_UNIT_TESTS)
    return ()
endif ()

##################################
set (COMMON_BASE_LIB_NAME "UnitTestCommonBase")
set (COMMON_BASE_SRC 
    "UnitTestCommonBase.cpp")

add_library(${COMMON_BASE_LIB_NAME} STATIC ${COMMON_BASE_SRC})
target_link_libraries(${COMMON_BASE_LIB_NAME} PUBLIC cc::cc_mqttsn)
target_include_directories(
    ${COMMON_BASE_LIB_NAME} 
        INTERFACE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        PUBLIC
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/client/lib/include>
)

##################################

function (cc_mqttsn_client_add_unit_test src test_lib)
    get_filename_component(name ${src} NAME_WE)
    cc_cxxtest_add_test (NAME cc.mqttsn.client.${name} SRC ${CMAKE_CURRENT_SOURCE_DIR}/${src})    
    target_link_libraries(cc.mqttsn.client.${name} PRIVATE ${test_lib} cxxtest::cxxtest)
endfunction ()

##################################

if (TARGET cc::cc_mqttsn_client)
    set (DEFAULT_BASE_LIB_NAME "UnitTestDefaultBase")
    set (DEFAULT_BASE_SRC 
        "default/UnitTestDefaultBase.cpp")    

    add_library(${DEFAULT_BASE_LIB_NAME} STATIC ${DEFAULT_BASE_SRC})
    target_link_libraries(${DEFAULT_BASE_LIB_NAME} PUBLIC ${COMMON_BASE_LIB_NAME} cc::cc_mqttsn_client)
    target_include_directories(
        ${DEFAULT_BASE_LIB_NAME} INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    )

    cc_mqttsn_client_add_unit_test(default/UnitTestGwDiscover.th ${DEFAULT_BASE_LIB_NAME})
    cc_mqttsn_client_add_unit_test(default/UnitTestConnect.th ${DEFAULT_BASE_LIB_NAME})
    cc_mqttsn_client_add_unit_test(default/UnitTestDisconnect.th ${DEFAULT_BASE_LIB_NAME})
    cc_mqttsn_client_add_unit_test(default/UnitTestPublish.th ${DEFAULT_BASE_LIB_NAME})
    cc_mqttsn_client_add_unit_test(default/UnitTestReceive.th ${DEFAULT_BASE_LIB_NAME})
    cc_mqttsn_client_add_unit_test(default/UnitTestSubscribe.th ${DEFAULT_BASE_LIB_NAME})
    cc_mqttsn_client_add_unit_test(default/UnitTestUnsubscribe.th ${DEFAULT_BASE_LIB_NAME})
    cc_mqttsn_client_add_unit_test(default/UnitTestWill.th ${DEFAULT_BASE_LIB_NAME})
    cc_mqttsn_client_add_unit_test(default/UnitTestSleep.th ${DEFAULT_BASE_LIB_NAME})
endif ()    

if (TARGET cc::cc_mqttsn_bm_client)
    set (BM_BASE_LIB_NAME "UnitTestBmBase")
    set (BM_BASE_SRC 
        "bm/UnitTestBmBase.cpp")    

    add_library(${BM_BASE_LIB_NAME} STATIC ${BM_BASE_SRC})
    target_link_libraries(${BM_BASE_LIB_NAME} PUBLIC ${COMMON_BASE_LIB_NAME} cc::cc_mqttsn_bm_client)
    target_include_directories(
        ${BM_BASE_LIB_NAME} INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    )

    cc_mqttsn_client_add_unit_test(bm/UnitTestBmClient.th ${BM_BASE_LIB_NAME})
    cc_mqttsn_client_add_unit_test(bm/UnitTestBmConnect.th ${BM_BASE_LIB_NAME})
    cc_mqttsn_client_add_unit_test(bm/UnitTestBmPublish.th ${BM_BASE_LIB_NAME})
endif ()  


if (TARGET cc::cc_mqttsn_qos1_client)
    set (QOS1_BASE_LIB_NAME "UnitTestQos1Base")
    set (QOS1_BASE_SRC 
        "qos1/UnitTestQos1Base.cpp")    

    add_library(${QOS1_BASE_LIB_NAME} STATIC ${QOS1_BASE_SRC})
    target_link_libraries(${QOS1_BASE_LIB_NAME} PUBLIC ${COMMON_BASE_LIB_NAME} cc::cc_mqttsn_qos1_client)
    target_include_directories(
        ${QOS1_BASE_LIB_NAME} INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    )

    cc_mqttsn_client_add_unit_test(qos1/UnitTestQos1Publish.th ${QOS1_BASE_LIB_NAME})
    cc_mqttsn_client_add_unit_test(qos1/UnitTestQos1Receive.th ${QOS1_BASE_LIB_NAME})
    cc_mqttsn_client_add_unit_test(qos1/UnitTestQos1Subscribe.th ${QOS1_BASE_LIB_NAME})
endif ()  

if (TARGET cc::cc_mqttsn_qos0_client)
    set (QOS0_BASE_LIB_NAME "UnitTestQos0Base")
    set (QOS0_BASE_SRC 
        "qos0/UnitTestQos0Base.cpp")    

    add_library(${QOS0_BASE_LIB_NAME} STATIC ${QOS0_BASE_SRC})
    target_link_libraries(${QOS0_BASE_LIB_NAME} PUBLIC ${COMMON_BASE_LIB_NAME} cc::cc_mqttsn_qos0_client)
    target_include_directories(
        ${QOS0_BASE_LIB_NAME} INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    )

    cc_mqttsn_client_add_unit_test(qos0/UnitTestQos0Publish.th ${QOS0_BASE_LIB_NAME})
    cc_mqttsn_client_add_unit_test(qos0/UnitTestQos0Receive.th ${QOS0_BASE_LIB_NAME})
    cc_mqttsn_client_add_unit_test(qos0/UnitTestQos0Subscribe.th ${QOS0_BASE_LIB_NAME})
endif ()  

if (TARGET cc::cc_mqttsn_no_gw_client)
    set (NO_GW_BASE_LIB_NAME "UnitTestNoGwBase")
    set (NO_GW_BASE_SRC 
        "no_gw/UnitTestNoGwBase.cpp")    

    add_library(${NO_GW_BASE_LIB_NAME} STATIC ${NO_GW_BASE_SRC})
    target_link_libraries(${NO_GW_BASE_LIB_NAME} PUBLIC ${COMMON_BASE_LIB_NAME} cc::cc_mqttsn_no_gw_client)
    target_include_directories(
        ${NO_GW_BASE_LIB_NAME} INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    )

    cc_mqttsn_client_add_unit_test(no_gw/UnitTestNoGwDiscover.th ${NO_GW_BASE_LIB_NAME})
endif () 
