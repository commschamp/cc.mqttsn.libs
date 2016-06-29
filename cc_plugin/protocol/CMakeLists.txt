if (NOT MQTTSN_CC_PROTOCOL_PLUGIN)
    return ()
endif ()
######################################################################

function (cc_plugin_mqttsn)
    set (name "mqttsn")
    
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

if (NOT "${MQTTSN_QT_DIR}" STREQUAL "")
    list (APPEND CMAKE_PREFIX_PATH ${MQTTSN_QT_DIR})
endif ()

find_package(Qt5Core REQUIRED)

if (NOT "${MQTTSN_CC_INSTALL_PATH}" STREQUAL "")
    list (APPEND CMAKE_PREFIX_PATH "${MQTTSN_CC_INSTALL_PATH}/cmake")
endif ()

find_package(CommsChampion)

if (NOT CC_COMMS_CHAMPION_FOUND)
    message (FATAL_ERROR "CommsChampion wasn't found!. Please set MQTTSN_CC_INSTALL_PATH to its installation path.")
endif ()

include_directories("${CC_INCLUDE_DIRS}")
link_directories("${CC_PLUGIN_LIBRARY_DIRS}")

if (CMAKE_COMPILER_IS_GNUCC)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftemplate-backtrace-limit=0")
endif ()

cc_plugin_mqttsn ()
