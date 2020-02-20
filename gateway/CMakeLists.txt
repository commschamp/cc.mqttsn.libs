if (NOT CC_MQTTSN_BUILD_GATEWAY)
    return ()
endif ()

######################################################################

set (CC_MQTT_EXTERNAL_DIR "${EXTERNALS_DIR}/cc.mqtt311.generated")
set (CC_MQTT_EXTERNAL_TGT "mqtt_external")
set (CC_MQTT_EXTERNAL_TGT_INCLUDE)

if ("${CC_MQTT311_GENERATED_INSTALL_DIR}" STREQUAL "")
    execute_process (
        COMMAND ${CMAKE_COMMAND} -E make_directory "${EXTERNALS_DIR}")
        
    include(ExternalProject)
    
    set (cc_mqtt_generated_tag "v0.15.2")
    set (cc_mqtt_main_dir "${CMAKE_CURRENT_BINARY_DIR}/cc.mqtt311.generated")
    set (cc_mqtt_src_dir "${CC_MQTT_EXTERNAL_DIR}")
    set (cc_mqtt_bin_dir "${cc_mqtt_main_dir}/build")
    set (cc_mqtt_install_dir "${cc_mqtt_main_dir}/install")
    
    ExternalProject_Add(
        "${CC_MQTT_EXTERNAL_TGT}"
        PREFIX "${cc_mqtt_bin_dir}"
        STAMP_DIR "${cc_mqtt_bin_dir}"    
        GIT_REPOSITORY "https://github.com/arobenko/cc.mqtt311.generated.git"
        GIT_TAG "${cc_mqtt_tag}"
        SOURCE_DIR "${cc_mqtt_src_dir}"
        CMAKE_ARGS 
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_INSTALL_PREFIX=${cc_mqtt_install_dir} 
            -DOPT_NO_COMMS=ON -DOPT_BUILD_TEST=OFF -DOPT_BUILD_PLUGIN=OFF
            -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD} 
            -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} 
        BINARY_DIR "${cc_mqtt_bin_dir}"
        UPDATE_DISCONNECTED 1
    )
    set (CC_MQTT_EXTERNAL_TGT_INCLUDE "${cc_mqtt_install_dir}/include") 
else ()
    list (APPEND CMAKE_PREFIX_PATH "${CC_MQTT311_GENERATED_INSTALL_DIR}")
    find_package(mqtt311 NO_MODULE)
    if (NOT TARGET cc::mqtt311)
        message (FATAL_ERROR "Failed to find external mqtt311 library")
    endif ()
endif ()

######################################################################
        
include_directories(
    BEFORE
    ${PROTOCOL_INC_DIR}
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

######################################################################

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
