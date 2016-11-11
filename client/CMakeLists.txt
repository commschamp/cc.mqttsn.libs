set (HEADER_TEMPL ${CMAKE_CURRENT_SOURCE_DIR}/templ/client.h.templ)
set (SRC_TEMPL ${CMAKE_CURRENT_SOURCE_DIR}/templ/client.cpp.templ)
set (TEMPL_PROCESS_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/script/ProcessTemplate.cmake)
set (DEFAULT_CLIENT_NAME "")
set (MQTTSN_CLIENT_LIB_NAME "mqttsn_client")

######################################################################

function (gen_lib_mqttsn_client name prot_opts client_opts inst)
    if (NOT ${name} STREQUAL "")
        set (name "${name}_")
    endif ()
    
    set (lib_name "mqttsn_${name}client")
    
    set (header_output ${CMAKE_CURRENT_BINARY_DIR}/${name}client.h)
    set (src_output ${CMAKE_CURRENT_BINARY_DIR}/${name}client.cpp)
    
    add_custom_command(
        OUTPUT "${header_output}"
        COMMAND ${CMAKE_COMMAND} 
            -DIN_FILE="${HEADER_TEMPL}"
            -DOUT_FILE="${header_output}"
            -DNAME="${name}"
            -DPROT_OPTS="${prot_opts}"
            -DCLIENT_OPTS="${client_opts}"
            -P ${TEMPL_PROCESS_SCRIPT}
        DEPENDS ${HEADER_TEMPL} ${TEMPL_PROCESS_SCRIPT}
    )
    
    set_source_files_properties(
        ${header_output}
        PROPERTIES GENERATED TRUE
    )
    
    set (header_tgt_name "${name}client.h.tgt") 
    add_custom_target(
        ${header_tgt_name}
        DEPENDS "${header_output}" ${HEADER_TEMPL} ${TEMPL_PROCESS_SCRIPT}
    )
    
    add_custom_command(
        OUTPUT "${src_output}"
        COMMAND ${CMAKE_COMMAND} 
            -DIN_FILE="${SRC_TEMPL}"
            -DOUT_FILE="${src_output}"
            -DNAME="${name}"
            -DPROT_OPTS="${prot_opts}"
            -DCLIENT_OPTS="${client_opts}"
            -P ${TEMPL_PROCESS_SCRIPT}
        DEPENDS ${SRC_TEMPL} ${TEMPL_PROCESS_SCRIPT}
    )
    
    set_source_files_properties(
        ${src_output}
        PROPERTIES GENERATED TRUE
    )
    
    set (src_tgt_name "${name}client.cpp.tgt") 
    add_custom_target(
        ${src_tgt_name}
        DEPENDS "${src_output}" ${SRC_TEMPL} ${TEMPL_PROCESS_SCRIPT}
    )
    
    add_library (${lib_name} STATIC ${src_output})
    target_include_directories(${lib_name} BEFORE PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
    set_target_properties(
        ${lib_name} PROPERTIES 
        POSITION_INDEPENDENT_CODE TRUE
    )
    add_dependencies(${lib_name} ${header_tgt_name} ${src_tgt_name})    
    
    if (inst)
        install (
            FILES ${header_output}
            DESTINATION ${INC_INSTALL_DIR}/mqttsn/client
        )
        
        install (
            TARGETS ${lib_name}
            DESTINATION ${LIB_INSTALL_DIR}
        )
    endif ()
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
    ${CMAKE_SOURCE_DIR}/protocol/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

install (
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/mqttsn
    DESTINATION ${INC_INSTALL_DIR}
)

add_subdirectory (src) 
add_subdirectory (test)
