option (CC_MQTTSN_BARE_METAL_IMAGE_TEST "Include build of bare metal image" OFF)

if ((NOT CC_MQTTSN_CLIENT_DEFAULT_LIB) AND
    (NOT CC_MQTTSN_BARE_METAL_IMAGE_TEST) AND
    ("${CC_MQTTSN_CUSTOM_CLIENT_CONFIG_FILES}" STREQUAL ""))
    return ()
endif ()

######################################################################

set (HEADER_TEMPL ${CMAKE_CURRENT_SOURCE_DIR}/templ/client.h.templ)
set (SRC_TEMPL ${CMAKE_CURRENT_SOURCE_DIR}/templ/client.cpp.templ)
set (TEMPL_PROCESS_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/script/ProcessTemplate.cmake)
set (DEFAULT_CLIENT_NAME "")
set (MQTTSN_CLIENT_LIB_NAME "cc_mqttsn_client")
set (MQTTSN_CLIENT_HEADER_TGT "client.h.tgt")

######################################################################

function (gen_lib_mqttsn_client name client_opts inst extra_flags)
    if (NOT ${name} STREQUAL "")
        set (name "${name}_")
    endif ()
    
    set (lib_name "cc_mqttsn_${name}client")
    
    set (header_output ${CMAKE_CURRENT_BINARY_DIR}/${name}client.h)
    set (src_output ${CMAKE_CURRENT_BINARY_DIR}/${name}client.cpp)
    
    add_custom_command(
        OUTPUT "${header_output}"
        COMMAND ${CMAKE_COMMAND} 
            -DIN_FILE="${HEADER_TEMPL}"
            -DOUT_FILE="${header_output}"
            -DNAME="${name}"
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
    
    message (STATUS "Defining library ${lib_name}") 
    add_library (${lib_name} STATIC ${src_output})
    target_include_directories(${lib_name} BEFORE PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
    target_link_libraries(${lib_name} PRIVATE cc::comms)

    set_target_properties(
        ${lib_name} PROPERTIES 
        POSITION_INDEPENDENT_CODE TRUE
    )
    add_dependencies(${lib_name} ${header_tgt_name} ${src_tgt_name})
    
    if (CMAKE_COMPILER_IS_GNUCC)
        list (APPEND extra_flags
            "-ftemplate-backtrace-limit=0"
            "-fno-rtti" 
            "-fno-exceptions" 
            "-fno-unwind-tables" 
            "-fno-threadsafe-statics"
        )
        
    endif ()
    
    if (NOT "${extra_flags}" STREQUAL "")
        string(REPLACE ";" " " extra_flags "${extra_flags}")
    
        set_target_properties(
            ${lib_name} PROPERTIES 
            COMPILE_FLAGS ${extra_flags}
        )
    endif ()

    if (TARGET cc::mqttsn)
        target_link_libraries(${lib_name} PRIVATE cc::mqttsn)
    endif ()

    if (NOT "${CC_MQTTSN_EXTERNAL_TGT_INCLUDE}" STREQUAL "")
        target_include_directories(${lib_name} PRIVATE ${CC_MQTTSN_EXTERNAL_TGT_INCLUDE})
    endif ()
    
    if (TARGET ${CC_MQTTSN_EXTERNAL_TGT})
        add_dependencies(${lib_name} ${CC_MQTTSN_EXTERNAL_TGT})
    endif ()

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

include_directories(
    BEFORE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

install (
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/mqttsn
    DESTINATION ${INC_INSTALL_DIR}
)

add_subdirectory (src) 
add_subdirectory (test)

######################################################################

find_package (Doxygen)
if (DOXYGEN_FOUND AND CC_MQTTSN_CLIENT_DEFAULT_LIB)
    set (doc_output_dir "${DOC_INSTALL_DIR}/mqttsn/client")
    make_directory (${doc_output_dir})
    
    set (match_str "OUTPUT_DIRECTORY[^\n]*")
    set (replacement_str "OUTPUT_DIRECTORY = ${doc_output_dir}")
    set (output_file "${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf")

    set (config_file "${CMAKE_CURRENT_SOURCE_DIR}/doc/doxygen.conf")
    file (READ ${config_file} config_text)
    string (REGEX REPLACE "${match_str}" "${replacement_str}" modified_config_text "${config_text}")
    file (WRITE "${output_file}" "${modified_config_text}")
    
    set (interface_doc_dir "${CMAKE_CURRENT_BINARY_DIR}/interface_doc")
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E make_directory ${interface_doc_dir})
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E copy 
        ${CMAKE_CURRENT_SOURCE_DIR}/doc/main.dox 
        ${CMAKE_CURRENT_SOURCE_DIR}/include/mqttsn/client/common.h 
        ${interface_doc_dir}/)
    
    add_custom_target ("doc_mqttsn_client"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/src/basic/client.h" ${interface_doc_dir}/ 
            COMMAND ${DOXYGEN_EXECUTABLE} ${output_file}
            WORKING_DIRECTORY ${interface_doc_dir})
            
    add_dependencies("doc_mqttsn_client" ${MQTTSN_CLIENT_HEADER_TGT})           
endif ()

