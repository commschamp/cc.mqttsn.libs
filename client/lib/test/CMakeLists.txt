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

function (cc_mqttsn_client_add_unit_test name test_lib)
    set (src ${CMAKE_CURRENT_SOURCE_DIR}/${name}.th)
    cc_cxxtest_add_test (NAME cc.mqttsn.client.${name} SRC ${src})    
    target_link_libraries(cc.mqttsn.client.${name} PRIVATE ${test_lib} cxxtest::cxxtest)
endfunction ()

##################################

if (TARGET cc::cc_mqttsn_client)
    set (DEFAULT_BASE_LIB_NAME "UnitTestDefaultBase")
    set (DEFAULT_BASE_SRC 
        "UnitTestDefaultBase.cpp")    

    add_library(${DEFAULT_BASE_LIB_NAME} STATIC ${DEFAULT_BASE_SRC})
    target_link_libraries(${DEFAULT_BASE_LIB_NAME} PUBLIC ${COMMON_BASE_LIB_NAME} cc::cc_mqttsn_client)
    target_include_directories(
        ${DEFAULT_BASE_LIB_NAME} INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    )

    cc_mqttsn_client_add_unit_test(UnitTestGwDiscover ${DEFAULT_BASE_LIB_NAME})
    cc_mqttsn_client_add_unit_test(UnitTestConnect ${DEFAULT_BASE_LIB_NAME})
    cc_mqttsn_client_add_unit_test(UnitTestDisconnect ${DEFAULT_BASE_LIB_NAME})
    cc_mqttsn_client_add_unit_test(UnitTestPublish ${DEFAULT_BASE_LIB_NAME})
#    cc_mqttsn_client_add_unit_test(UnitTestReceive ${DEFAULT_BASE_LIB_NAME})
    cc_mqttsn_client_add_unit_test(UnitTestSubscribe ${DEFAULT_BASE_LIB_NAME})
    cc_mqttsn_client_add_unit_test(UnitTestUnsubscribe ${DEFAULT_BASE_LIB_NAME})
    cc_mqttsn_client_add_unit_test(UnitTestWill ${DEFAULT_BASE_LIB_NAME})
endif ()    

# TODO
return ()

if (TARGET cc::cc_mqttsn_bm_client)
    set (BM_BASE_LIB_NAME "UnitTestBmBase")
    set (BM_BASE_SRC 
        "UnitTestBmBase.cpp")    

    add_library(${BM_BASE_LIB_NAME} STATIC ${BM_BASE_SRC})
    target_link_libraries(${BM_BASE_LIB_NAME} PUBLIC ${COMMON_BASE_LIB_NAME} cc::cc_mqttsn_bm_client)
    target_include_directories(
        ${BM_BASE_LIB_NAME} INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    )

    cc_mqttsn_client_add_unit_test(UnitTestBmClient ${BM_BASE_LIB_NAME})
    cc_mqttsn_client_add_unit_test(UnitTestBmConnect ${BM_BASE_LIB_NAME})
    cc_mqttsn_client_add_unit_test(UnitTestBmPublish ${BM_BASE_LIB_NAME})
    cc_mqttsn_client_add_unit_test(UnitTestBmReceive ${BM_BASE_LIB_NAME})
endif ()  

if (TARGET cc::cc_mqttsn_qos1_client)
    set (QOS1_BASE_LIB_NAME "UnitTestQos1Base")
    set (QOS1_BASE_SRC 
        "UnitTestQos1Base.cpp")    

    add_library(${QOS1_BASE_LIB_NAME} STATIC ${QOS1_BASE_SRC})
    target_link_libraries(${QOS1_BASE_LIB_NAME} PUBLIC ${COMMON_BASE_LIB_NAME} cc::cc_mqttsn_qos1_client)
    target_include_directories(
        ${QOS1_BASE_LIB_NAME} INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    )

    cc_mqttsn_client_add_unit_test(UnitTestQos1Publish ${QOS1_BASE_LIB_NAME})
    cc_mqttsn_client_add_unit_test(UnitTestQos1Receive ${QOS1_BASE_LIB_NAME})
    cc_mqttsn_client_add_unit_test(UnitTestQos1Subscribe ${QOS1_BASE_LIB_NAME})
endif ()  

if (TARGET cc::cc_mqttsn_qos0_client)
    set (QOS0_BASE_LIB_NAME "UnitTestQos0Base")
    set (QOS0_BASE_SRC 
        "UnitTestQos0Base.cpp")    

    add_library(${QOS0_BASE_LIB_NAME} STATIC ${QOS0_BASE_SRC})
    target_link_libraries(${QOS0_BASE_LIB_NAME} PUBLIC ${COMMON_BASE_LIB_NAME} cc::cc_mqttsn_qos0_client)
    target_include_directories(
        ${QOS0_BASE_LIB_NAME} INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    )

    cc_mqttsn_client_add_unit_test(UnitTestQos0Publish ${QOS0_BASE_LIB_NAME})
    cc_mqttsn_client_add_unit_test(UnitTestQos0Receive ${QOS0_BASE_LIB_NAME})
    cc_mqttsn_client_add_unit_test(UnitTestQos0Subscribe ${QOS0_BASE_LIB_NAME})
endif ()  
