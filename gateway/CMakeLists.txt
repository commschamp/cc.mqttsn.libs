if (NOT CC_MQTTSN_GATEWAY_LIB)
    return ()
endif ()

set (MQTTSN_GATEWAY_LIB_NAME "cc_mqttsn_gateway")

add_subdirectory (lib)
add_subdirectory (app)