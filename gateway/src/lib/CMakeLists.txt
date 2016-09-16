function (lib_mqttsn_gateway)
    set (name "${MQTTSN_GATEWAY_LIB_NAME}")

    set (src
        Gateway.cpp
        GatewayImpl.cpp
        Session.cpp
        SessionImpl.cpp
        session_op/Connect.cpp
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