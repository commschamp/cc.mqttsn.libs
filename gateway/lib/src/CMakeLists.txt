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
        session_op/Encapsulate.cpp
        session_op/Forward.cpp
        session_op/Ping.cpp
        session_op/PubRecv.cpp
        session_op/PubSend.cpp
        session_op/WillUpdate.cpp
    )    
    
    add_library (${name} ${src})
    add_library (cc::${name} ALIAS ${name})
    
    target_link_libraries(${name} PRIVATE cc::cc_mqtt311 cc::cc_mqttsn cc::comms)

    set_target_properties(
        ${name} PROPERTIES 
        INTERFACE_LINK_LIBRARIES ""
    )

    if (CC_MQTTSN_GATEWAY_LIB_FORCE_PIC)
        set_property(TARGET ${name} PROPERTY POSITION_INDEPENDENT_CODE ON)
    endif ()    

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

    include(CMakePackageConfigHelpers)
    file (READ "${CMAKE_CURRENT_SOURCE_DIR}/../include/cc_mqttsn_gateway/version.h" version_file)
    string (REGEX MATCH "CC_MQTTSN_GW_MAJOR_VERSION ([0-9]*)U*" _ ${version_file})
    set (major_ver ${CMAKE_MATCH_1})
    string (REGEX MATCH "CC_MQTTSN_GW_MINOR_VERSION ([0-9]*)U*" _ ${version_file})
    set (minor_ver ${CMAKE_MATCH_1})
    string (REGEX MATCH "CC_MQTTSN_GW_PATCH_VERSION ([0-9]*)U*" _ ${version_file})
    set (patch_ver ${CMAKE_MATCH_1})
    set (CC_MQTTSN_GW_VERSION "${major_ver}.${minor_ver}.${patch_ver}")
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/${name}ConfigVersion.cmake
        VERSION ${CC_MQTTSN_GW_VERSION}
        COMPATIBILITY AnyNewerVersion)  

    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/${name}ConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/${name}/cmake
    )              
    
endfunction()

######################################################################

lib_mqttsn_gateway ()

