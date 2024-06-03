if ((NOT CC_MQTTSN_CLIENT_DEFAULT_LIB) OR (NOT CC_MQTTSN_CLIENT_APPS))
    return ()
endif ()

add_subdirectory (sub)
add_subdirectory (pub)