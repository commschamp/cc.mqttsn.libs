if (NOT MQTTSN_CLIENT_LIB)
    return ()
endif ()

set (MQTTSN_CLIENT_LIB_NAME "mqttsn_client")
######################################################################

function (lib_mqttsn_client)
    set (name "${MQTTSN_CLIENT_LIB_NAME}")
    
    set (src
        src/client.cpp
    )
    
    add_library (${name} STATIC ${src})
    
    install (
        TARGETS ${name}
        DESTINATION ${LIB_INSTALL_DIR})

endfunction()

######################################################################

if (NOT "${MQTTSN_CC_INSTALL_PATH}" STREQUAL "")
    list (APPEND CMAKE_PREFIX_PATH "${MQTTSN_CC_INSTALL_PATH}/cmake")
endif ()

find_package(CommsChampion)

if (NOT CC_COMMS_FOUND)
    message (FATAL_ERROR "Comms library wasn't found!. Please set MQTTSN_CC_INSTALL_PATH to its installation path.")
endif ()

include_directories("${CC_INCLUDE_DIRS}")

if (CMAKE_COMPILER_IS_GNUCC)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftemplate-backtrace-limit=0")
endif ()

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/protocol/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if (CMAKE_CROSSCOMPILING)
    add_definitions("-DNOSTDLIB")
    if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CROSSCOMPILING)
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-threadsafe-statics -fno-exceptions -fno-unwind-tables -fno-rtti -nostdlib") 
    endif ()
endif ()


lib_mqttsn_client ()

add_subdirectory (test)
