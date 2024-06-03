if (NOT CC_MQTTSN_CLIENT_APPS)
    return ()
endif ()

if (NOT CC_MQTTSN_CLIENT_DEFAULT_LIB)
    message (WARNING "Cannot compile applications due to missing default library")
    return ()
endif ()

find_package (Threads REQUIRED)
find_package (Boost REQUIRED COMPONENTS system program_options)

set (EXTRA_BOOST_TARGETS)
if (WIN32)
    find_package (Boost REQUIRED COMPONENTS date_time regex)
    set (EXTRA_BOOST_TARGETS Boost::date_time Boost::regex)
endif ()

set (COMMON_APPS_LIB "cc_mqttsn_client_apps_lib")

add_subdirectory (common)
add_subdirectory (gw_discover)
#add_subdirectory (pub)
#add_subdirectory (sub)