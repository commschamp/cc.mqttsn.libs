function (lib_mqttsn_gateway)
    set (name "${MQTTSN_GATEWAY_LIB_NAME}")

    set (src
        Gateway.cpp
        GatewayImpl.cpp
        ConfigParser.cpp
        ConfigParserImpl.cpp
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

    install (
        TARGETS ${name}
        DESTINATION ${LIB_INSTALL_DIR}
    )
    
endfunction()

######################################################################

add_definitions(
    -DMQTTSN_GATEWAY_LIB_EXPORT
)

include_directories(
    BEFORE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

lib_mqttsn_gateway ()