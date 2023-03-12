function (bin_gateway_udp)
    set (name "cc_mqttsn_gateway_udp")
    
    if (NOT Qt5Core_FOUND)
        message(WARNING "Can NOT build ${name} due to missing Qt5Core library")
        return()
    endif ()
    
    if (NOT Qt5Network_FOUND)
        message(WARNING "Can NOT build ${name} due to missing Qt5Network library")
        return()
    endif ()
    
    
    set (src
        main.cpp
        Mgr.cpp
        GatewayWrapper.cpp
        SessionWrapper.cpp
    )
    
    qt5_wrap_cpp(
        moc
        Mgr.h
        GatewayWrapper.h
        SessionWrapper.h
    )
    
    #qt5_add_resources(resources ${CMAKE_CURRENT_SOURCE_DIR}/ui.qrc)

    add_executable(${name} ${src} ${moc})
    target_link_libraries(${name} ${MQTTSN_GATEWAY_LIB_NAME} Qt5::Network Qt5::Core)
    
    install (
        TARGETS ${name}
        DESTINATION ${CMAKE_INSTALL_BINDIR})
        
endfunction ()

###########################################################

find_package(Qt5Core)
find_package(Qt5Network)

bin_gateway_udp()
