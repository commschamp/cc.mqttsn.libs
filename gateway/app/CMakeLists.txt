if (NOT CC_MQTTSN_BUILD_GATEWAY_APPS)
    return ()
endif ()

#add_subdirectory (udp)
add_subdirectory (gateway)