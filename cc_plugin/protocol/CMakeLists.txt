set (THIS_PLUGIN_NAME "cc_mqttsn_protocol")

######################################################################

function (cc_plugin_mqttsn_protocol)
    set (name "${THIS_PLUGIN_NAME}")
    
    set (meta_file "${CMAKE_CURRENT_SOURCE_DIR}/mqttsn.json")
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
        Protocol.cpp
        TransportMessage.cpp
        field.cpp
        message/Advertise.cpp
        message/Searchgw.cpp
        message/Gwinfo.cpp
        message/Connect.cpp
        message/Connack.cpp
        message/Willtopicreq.cpp
        message/Willtopic.cpp
        message/Willmsgreq.cpp
        message/Willmsg.cpp
        message/Register.cpp
        message/Regack.cpp
        message/Publish.cpp
        message/Puback.cpp
        message/Pubcomp.cpp
        message/Pubrec.cpp
        message/Pubrel.cpp
        message/Subscribe.cpp
        message/Suback.cpp
        message/Unsubscribe.cpp
        message/Unsuback.cpp
        message/Pingreq.cpp
        message/Pingresp.cpp
        message/Disconnect.cpp
        message/Willtopicupd.cpp
        message/Willtopicresp.cpp
        message/Willmsgupd.cpp
        message/Willmsgresp.cpp
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

endfunction()

######################################################################

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/protocol/include)

if (CMAKE_COMPILER_IS_GNUCC)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftemplate-backtrace-limit=0")
endif ()

cc_plugin_mqttsn_protocol ()

if (CC_MQTTSN_FULL_SOLUTION)
    add_dependencies(${THIS_PLUGIN_NAME} ${CC_EXTERNAL_TGT})
endif ()
