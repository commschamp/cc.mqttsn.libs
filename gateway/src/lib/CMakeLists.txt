function (lib_mqttsn_gateway)
    set (name "${MQTTSN_GATEWAY_LIB_NAME}")

    set (src
        gateway_all.c
        gateway_all.cpp
        Gateway.cpp
        GatewayImpl.cpp
        Config.cpp
        ConfigImpl.cpp
        Session.cpp
        SessionImpl.cpp
        RegMgr.cpp
        SessionOp.cpp
        session_op/Connect.cpp
        session_op/Disconnect.cpp
        session_op/Asleep.cpp
        session_op/AsleepMonitor.cpp
        session_op/PubRecv.cpp
        session_op/PubSend.cpp
        session_op/Forward.cpp
        session_op/WillUpdate.cpp
    )    
    
    add_library (${name} STATIC ${src})
    if (TARGET cc::mqtt311)
        target_link_libraries(${name} PRIVATE cc::mqtt311)    
    endif ()
    
    target_link_libraries(${name} PRIVATE cc::comms)
    
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
    else ()
        target_link_libraries(${name} PRIVATE cc::mqtt311)
    endif ()
    
    if (TARGET ${CC_MQTTSN_EXTERNAL_TGT})
        add_dependencies(${name} ${CC_MQTTSN_EXTERNAL_TGT})
    else ()
        target_link_libraries(${name} PRIVATE cc::mqttsn)
    endif ()

    install (
        TARGETS ${name}
        DESTINATION ${LIB_INSTALL_DIR}
    )
    
endfunction()

######################################################################

include_directories(
    BEFORE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

lib_mqttsn_gateway ()

