if (NOT CC_MQTTSN_CLIENT_DEFAULT_LIB)
    return ()
endif ()

add_subdirectory (sub)
add_subdirectory (pub)