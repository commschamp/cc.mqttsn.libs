function (bin_gateway_udp)
    set (name "mqttsn_gateway_udp")
    
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
    target_link_libraries(${name} ${MQTTSN_GATEWAY_LIB_NAME})
    qt5_use_modules(${name} Network Core)
    
    install (
        TARGETS ${name}
        DESTINATION ${BIN_INSTALL_DIR})
        
endfunction ()

###########################################################

if (NOT "${MQTTSN_QT_DIR}" STREQUAL "")
    list (APPEND CMAKE_PREFIX_PATH ${MQTTSN_QT_DIR})
endif ()

find_package(Qt5Core)
find_package(Qt5Network)

include_directories (
    ${CMAKE_CURRENT_SOURCE_DIR}
)

bin_gateway_udp()
