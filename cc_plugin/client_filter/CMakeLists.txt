
function (cc_plugin_mqttsn_client_filter)
    set (name "mqttsn_client_filter")
    
    set (meta_file "${CMAKE_CURRENT_SOURCE_DIR}/mqttsn_client_filter.json")
    set (stamp_file "${CMAKE_CURRENT_BINARY_DIR}/refresh_stamp.txt")
    
    set (refresh_plugin_header TRUE)
    if ((NOT EXISTS ${stamp_file}) OR (${meta_file} IS_NEWER_THAN ${stamp_file}))
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_SOURCE_DIR}/Plugin.h)
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E touch ${stamp_file})
    endif ()
    
    
    set (src
        Plugin.cpp
    )
    
    set (hdr
        Plugin.h
    )
    
    
    qt5_wrap_cpp(
        moc
        ${hdr}
    )
    
    add_library (${name} SHARED ${src} ${moc})
    target_link_libraries (${name} ${CC_PLUGIN_LIBRARIES})
    qt5_use_modules(${name} Core)
    
    install (
        TARGETS ${name}
        DESTINATION ${PLUGIN_INSTALL_DIR})

    if (MQTTSN_CC_PLUGIN_COPY_TO_CC_INSTALL_PATH)        
        install (
            TARGETS ${name}
            DESTINATION ${CC_PLUGIN_DIR})
     endif ()
    
endfunction()

######################################################################

include_directories(
    ${CMAKE_BINARY_DIR}/client/src/basic
)

cc_plugin_mqttsn_client_filter ()
