if (CMAKE_CROSSCOMPILING)
    return()
endif ()

if (NOT MQTTSN_CC_PLUGINS)
    return()
endif ()    

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

add_subdirectory (protocol)
add_subdirectory (client_filter)