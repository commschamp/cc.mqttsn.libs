function (bin_cc_mqttsn_gateway_app)
    set (name "cc_mqttsn_gateway_app")
    
    set (src
        main.cpp
        GatewayApp.cpp
        GatewayIoBrokerSocket.cpp
        GatewayIoBrokerSocket_Tcp.cpp
        GatewayIoClientAcceptor.cpp
        GatewayIoClientAcceptor_Udp.cpp
        GatewayIoClientSocket.cpp
        GatewayIoClientSocket_Udp.cpp
        GatewayLogger.cpp
        GatewayProgramOptions.cpp
        GatewaySession.cpp
        GatewayWrapper.cpp
    )
    
    add_executable(${name} ${src})
    target_link_libraries(${name} cc::${MQTTSN_GATEWAY_LIB_NAME} Boost::system Boost::program_options ${EXTRA_BOOST_TARGETS} ${CMAKE_THREAD_LIBS_INIT})
    
    install (
        TARGETS ${name}
        DESTINATION ${CMAKE_INSTALL_BINDIR})

    install (
        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/etc/
        DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/cc_mqttsn_gateway
    )        
        
endfunction ()

###########################################################

find_package (Threads REQUIRED)
find_package(Boost COMPONENTS system program_options)

set (EXTRA_BOOST_TARGETS)
if (WIN32)
    find_package (Boost REQUIRED COMPONENTS date_time regex)
    set (EXTRA_BOOST_TARGETS Boost::date_time Boost::regex)
endif ()

bin_cc_mqttsn_gateway_app()
