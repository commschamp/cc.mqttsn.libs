if (NOT CC_MQTTSN_BUILD_GATEWAY)
    return ()
endif ()
        
if (NOT CC_COMMS_FOUND)
    message (WARNING "Comms library wasn't found, cannot build gateway library and/or application.")
    return ()
endif ()

if (NOT CC_MQTT_FOUND)
    message (WARNING "MQTT protocol library wasn't found, cannot build gateway library and/or application.")
    return ()
endif ()

include_directories(
    BEFORE
    ${CMAKE_SOURCE_DIR}/protocol/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if (CMAKE_COMPILER_IS_GNUCC)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftemplate-backtrace-limit=0")
endif ()

set (MQTTSN_GATEWAY_LIB_NAME "cc_mqttsn_gateway")

add_subdirectory (src)
add_subdirectory (test)

install (
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/mqttsn
    DESTINATION ${INC_INSTALL_DIR}
)

install (
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/etc/
    DESTINATION ${ETC_INSTALL_DIR}/mqttsn/
)

find_package (Doxygen)
if (DOXYGEN_FOUND)
    set (doc_output_dir "${DOC_INSTALL_DIR}/mqttsn/gateway")
    make_directory (${doc_output_dir})
    
    set (match_str "OUTPUT_DIRECTORY[^\n]*")
    set (replacement_str "OUTPUT_DIRECTORY = ${doc_output_dir}")
    set (output_file "${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf")

    set (config_file "${CMAKE_CURRENT_SOURCE_DIR}/doc/doxygen.conf")
    file (READ ${config_file} config_text)
    string (REGEX REPLACE "${match_str}" "${replacement_str}" modified_config_text "${config_text}")
    file (WRITE "${output_file}" "${modified_config_text}")

    add_custom_target ("doc_mqttsn_gateway"
            COMMAND ${DOXYGEN_EXECUTABLE} ${output_file}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif ()