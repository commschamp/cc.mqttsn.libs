function (lib_mqttsn_gateway)
    set (name "${MQTTSN_GATEWAY_LIB_NAME}")

    set (src
        Gateway.cpp
        GatewayImpl.cpp
        Session.cpp
    )    
    
    add_library (${name} STATIC ${src})

    install (
        TARGETS ${name}
        DESTINATION ${LIB_INSTALL_DIR}
    )
    
endfunction()

######################################################################

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
)

if (CMAKE_COMPILER_IS_GNUCC)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftemplate-backtrace-limit=0")
endif ()

add_definitions(
    -DMQTTSN_GATEWAY_LIB_EXPORT
)

lib_mqttsn_gateway ()