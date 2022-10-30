function (bin_sub_udp)
    set (name "cc_mqttsn_sub_udp")
    
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
        Sub.cpp
    )
    
    qt5_wrap_cpp(
        moc
        Sub.h
    )
    
    #qt5_add_resources(resources ${CMAKE_CURRENT_SOURCE_DIR}/ui.qrc)

    add_executable(${name} ${src} ${moc})
    target_link_libraries(${name} ${MQTTSN_CLIENT_LIB_NAME} Qt5::Network Qt5::Core)
    
    install (
        TARGETS ${name}
        DESTINATION ${BIN_INSTALL_DIR})
        
endfunction ()

###########################################################

find_package(Qt5Core)
find_package(Qt5Network)

include_directories (
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}/client/src/basic
)

bin_sub_udp()
