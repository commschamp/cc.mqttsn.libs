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
    
    target_link_libraries(${name} PRIVATE cc::cc_mqtt311 cc::cc_mqttsn cc::comms)

    set_target_properties(
        ${name} PROPERTIES 
        INTERFACE_LINK_LIBRARIES ""
    )

    target_include_directories(
        ${name} BEFORE
            PUBLIC
                $<INSTALL_INTERFACE:include>
                $<BUILD_INTERFACE:${GATEWAY_INC_DIR}>
            PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR})        
    
    install (
        TARGETS ${name}
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        EXPORT ${name}Config
    )

    install(EXPORT ${name}Config NAMESPACE cc::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/${name}/cmake
    )       
    
endfunction()

######################################################################

lib_mqttsn_gateway ()

