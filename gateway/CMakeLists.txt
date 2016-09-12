if (CMAKE_CROSSCOMPILING)
    return()
endif ()

if (NOT "${MQTTSN_CC_INSTALL_PATH}" STREQUAL "")
    list (APPEND CMAKE_PREFIX_PATH "${MQTTSN_CC_INSTALL_PATH}/cmake")
endif ()

find_package(CommsChampion)

if (NOT CC_COMMS_FOUND)
    message (FATAL_ERROR "Comms library wasn't found!. Please set MQTTSN_CC_INSTALL_PATH to its installation path.")
endif ()

include_directories(
    "${CC_INCLUDE_DIRS}"
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
