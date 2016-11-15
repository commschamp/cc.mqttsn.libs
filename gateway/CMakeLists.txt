if (NOT CC_COMMS_FOUND)
    message (WARNING "Comms library wasn't found, cannot build gateway library and/or application.")
    return ()
endif ()

if (NOT CC_MQTT_FOUND)
    message (WARNING "MQTT protocol library wasn't found, cannot build gateway library and/or application.")
    return ()
endif ()

include_directories(
    ${CMAKE_SOURCE_DIR}/protocol/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if (CMAKE_COMPILER_IS_GNUCC)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftemplate-backtrace-limit=0")
endif ()

set (MQTTSN_GATEWAY_LIB_NAME "mqttsn_gateway")

add_subdirectory (src)
add_subdirectory (test)

install (
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/mqttsn
    DESTINATION ${INC_INSTALL_DIR}
)

install (
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/etc/
    DESTINATION ${ETC_INSTALL_DIR}/mqttsn/
)
