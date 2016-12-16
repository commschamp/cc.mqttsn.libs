if (NOT CC_MQTTSN_BUILD_PLUGINS)
    return()
endif ()

find_package(Qt5Core)

if (NOT Qt5Core_FOUND)
    message (WARNING "Can NOT compile commsChampion plugins due to missing QT5 Core library")
    return ()
endif ()

if (NOT CC_COMMS_CHAMPION_FOUND)
    message (WARNING "Can NOT compile mqtt protocol plugins due to missing CommsChampion library")
    return ()
endif ()
    
add_subdirectory (protocol)
add_subdirectory (client_filter)