if ((NOT CC_MQTTSN_CLIENT_DEFAULT_LIB) OR
    (CC_MQTTSN_CUSTOM_CLIENT_CONFIG_FILES STREQUAL ""))
    return()
endif ()


######################################################################

set (HEADER_TEMPL ${CMAKE_CURRENT_SOURCE_DIR}/templ/client.h.templ)
set (SRC_TEMPL ${CMAKE_CURRENT_SOURCE_DIR}/templ/client.cpp.templ)
set (C_TEMPL ${CMAKE_CURRENT_SOURCE_DIR}/templ/client.c.templ)
set (CONFIG_TEMPL ${CMAKE_CURRENT_SOURCE_DIR}/templ/Config.h.templ)
set (PROT_OPTS_TEMPL ${CMAKE_CURRENT_SOURCE_DIR}/templ/ProtocolOptions.h.templ)
set (TEMPL_PROCESS_SCRIPT ${PROJECT_SOURCE_DIR}/cmake/ProcessTemplate.cmake)
set (WRITE_CONFIG_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/script/WriteConfigHeader.cmake)
set (WRITE_PROT_OPTS_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/script/WriteProtocolOptions.cmake)
set (DEFAULT_CONFIG_VARS_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/script/DefineDefaultConfigVars.cmake)
set (DEFAULT_CLIENT_DIR_NAME "default")
set (COMMON_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

######################################################################

function (gen_lib_mqttsn_client config_file)
    if (NOT "${config_file}" STREQUAL "")
        include (${config_file})
    endif()

    set (name "${CC_MQTTSN_CLIENT_CUSTOM_NAME}")
    if ("${CC_MQTTSN_CLIENT_CUSTOM_NAME}" STREQUAL "")
        set (dir ${DEFAULT_CLIENT_DIR_NAME})
        set (lib_name "cc_mqttsn_client")
    else ()
        set (dir "${CC_MQTTSN_CLIENT_CUSTOM_NAME}")
        set (name "${name}_")
        set (lib_name "cc_mqttsn_${name}client")
    endif ()
    
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/${dir})    

    set (header_output ${CMAKE_CURRENT_BINARY_DIR}/${dir}/${name}client.h)
    set (src_output ${CMAKE_CURRENT_BINARY_DIR}/${dir}/${name}client.cpp)
    set (c_output ${CMAKE_CURRENT_BINARY_DIR}/${dir}/${name}client.c)
    set (config_output ${CMAKE_CURRENT_BINARY_DIR}/${dir}/Config.h)
    set (prot_opts_output ${CMAKE_CURRENT_BINARY_DIR}/${dir}/ProtocolOptions.h)

    # ---------------------------------    
    
    add_custom_command(
        OUTPUT "${header_output}"
        COMMAND ${CMAKE_COMMAND} 
            -DIN_FILE="${HEADER_TEMPL}"
            -DOUT_FILE="${header_output}"
            -DNAME="${name}"
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

    # ---------------------------------
    
    add_custom_command(
        OUTPUT "${src_output}"
        COMMAND ${CMAKE_COMMAND} 
            -DIN_FILE="${SRC_TEMPL}"
            -DOUT_FILE="${src_output}"
            -DNAME="${name}"
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

    # ---------------------------------

    add_custom_command(
        OUTPUT "${c_output}"
        COMMAND ${CMAKE_COMMAND} 
            -DIN_FILE="${C_TEMPL}"
            -DOUT_FILE="${c_output}"
            -DNAME="${name}"
            -P ${TEMPL_PROCESS_SCRIPT}
        DEPENDS ${C_TEMPL} ${TEMPL_PROCESS_SCRIPT}
    )
    
    set_source_files_properties(
        ${c_output}
        PROPERTIES GENERATED TRUE
    )
    
    set (c_tgt_name "${name}client.c.tgt") 
    add_custom_target(
        ${c_tgt_name}
        DEPENDS "${c_output}" ${C_TEMPL} ${TEMPL_PROCESS_SCRIPT}
    )

    # ---------------------------------    

    add_custom_command(
        OUTPUT "${config_output}"
        COMMAND ${CMAKE_COMMAND} 
            -DCMAKE_CONFIG_FILE="${config_file}"
            -DCMAKE_DEFAULT_CONFIG_VARS="${DEFAULT_CONFIG_VARS_SCRIPT}"
            -DCONFIG_HEADER_TEMPL="${CONFIG_TEMPL}"
            -DOUT_FILE="${config_output}"
            -P ${WRITE_CONFIG_SCRIPT}
        DEPENDS ${config_file} ${CONFIG_TEMPL} ${WRITE_CONFIG_SCRIPT} ${DEFAULT_CONFIG_VARS_SCRIPT}
    )
    
    set_source_files_properties(
        ${config_output}
        PROPERTIES GENERATED TRUE
    )
    
    set (config_tgt_name "${name}Config.h.tgt") 
    add_custom_target(
        ${config_tgt_name}
        DEPENDS "${config_output}" ${CONFIG_TEMPL} ${WRITE_CONFIG_SCRIPT}
    )    

    # ---------------------------------

    add_custom_command(
        OUTPUT "${prot_opts_output}"
        COMMAND ${CMAKE_COMMAND} 
            -DCMAKE_CONFIG_FILE="${config_file}"
            -DCMAKE_DEFAULT_CONFIG_VARS="${DEFAULT_CONFIG_VARS_SCRIPT}"
            -DPROT_OPTS_HEADER_TEMPL="${PROT_OPTS_TEMPL}"
            -DOUT_FILE="${prot_opts_output}"
            -P ${WRITE_PROT_OPTS_SCRIPT}
        DEPENDS ${config_file} ${PROT_OPTS_TEMPL} ${WRITE_PROT_OPTS_SCRIPT}
    )
    
    set_source_files_properties(
        ${prot_opts_output}
        PROPERTIES GENERATED TRUE
    )
    
    set (prot_opts_tgt_name "${name}ProtocolOptions.h.tgt") 
    add_custom_target(
        ${prot_opts_tgt_name}
        DEPENDS "${prot_opts_output}" ${PROT_OPTS_TEMPL} ${WRITE_PROT_OPTS_SCRIPT}
    )    

    # ---------------------------------    
    
    message (STATUS "Defining library ${lib_name}") 
    set (src
        src/op/Op.cpp
        src/op/SearchOp.cpp
        src/ClientImpl.cpp
        src/TimerMgr.cpp
    )
    add_library (${lib_name} ${src} ${src_output} ${c_output})
    add_library (cc::${lib_name} ALIAS ${lib_name})
    target_link_libraries(${lib_name} PRIVATE cc::cc_mqttsn cc::comms)
    target_include_directories(
            ${lib_name} BEFORE
                PUBLIC
                    $<INSTALL_INTERFACE:include>
                    $<BUILD_INTERFACE:${COMMON_INC_DIR}>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/${dir}>
                PRIVATE 
                    ${CMAKE_CURRENT_SOURCE_DIR}/src)    

    set_target_properties(
        ${lib_name} PROPERTIES 
        INTERFACE_LINK_LIBRARIES ""
    )
    add_dependencies(${lib_name} ${header_tgt_name} ${src_tgt_name} ${c_tgt_name} ${config_tgt_name} ${prot_opts_tgt_name})

    if (CC_MQTTSN_CLIENT_LIB_FORCE_PIC)
        set_property(TARGET ${lib_name} PROPERTY POSITION_INDEPENDENT_CODE ON)
    endif ()

    file (READ "${CMAKE_CURRENT_SOURCE_DIR}/include/cc_mqttsn_client/common.h" version_file)
    string (REGEX MATCH "CC_MQTTSN_CLIENT_MAJOR_VERSION ([0-9]*)U*" _ ${version_file})
    set (major_ver ${CMAKE_MATCH_1})
    string (REGEX MATCH "CC_MQTTSN_CLIENT_MINOR_VERSION ([0-9]*)U*" _ ${version_file})
    set (minor_ver ${CMAKE_MATCH_1})
    string (REGEX MATCH "CC_MQTTSN_CLIENT_PATCH_VERSION ([0-9]*)U*" _ ${version_file})
    set (patch_ver ${CMAKE_MATCH_1})
    set (CC_MQTTSN_CLIENT_VERSION "${major_ver}.${minor_ver}.${patch_ver}")

    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/${lib_name}ConfigVersion.cmake
        VERSION ${CC_MQTTSN_CLIENT_VERSION}
        COMPATIBILITY AnyNewerVersion)    

    install (
        FILES ${CMAKE_CURRENT_BINARY_DIR}/${lib_name}ConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/${lib_name}/cmake
    )        
    
    install (
        FILES ${header_output}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cc_mqttsn_client
    )
    
    install (
        TARGETS ${lib_name}
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        EXPORT ${lib_name}Config
    )

    install(EXPORT ${lib_name}Config NAMESPACE cc::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/${lib_name}/cmake
    )        

    if (DOXYGEN_FOUND)
        set (doc_output_dir "${CMAKE_INSTALL_FULL_DATAROOTDIR}/doc/${lib_name}")
        make_directory (${doc_output_dir})
        
        set (match_str "OUTPUT_DIRECTORY[^\n]*")
        set (replacement_str "OUTPUT_DIRECTORY = ${doc_output_dir}")
        set (output_file "${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf")

        if (NOT "${CC_MQTTSN_CLIENT_CUSTOM_NAME}" STREQUAL "")
            set (output_file "${CMAKE_CURRENT_BINARY_DIR}/doxygen_${CC_MQTTSN_CLIENT_CUSTOM_NAME}.conf")
        endif ()        

        set (config_file "${CMAKE_CURRENT_SOURCE_DIR}/doxygen/doxygen.conf")
        file (READ ${config_file} config_text)
        string (REGEX REPLACE "${match_str}" "${replacement_str}" modified_config_text "${config_text}")
        file (WRITE "${output_file}" "${modified_config_text}")
        
        set (interface_doc_dir "${CMAKE_CURRENT_BINARY_DIR}/interface_doc")
        if (NOT "${CC_MQTTSN_CLIENT_CUSTOM_NAME}" STREQUAL "")
            set (interface_doc_dir "${interface_doc_dir}_${CC_MQTTSN_CLIENT_CUSTOM_NAME}")
        endif ()           

        execute_process(
            COMMAND ${CMAKE_COMMAND} -E make_directory ${interface_doc_dir})

        set (doc_tgt_name "doc_mqttsn_client")
        if (NOT "${CC_MQTTSN_CLIENT_CUSTOM_NAME}" STREQUAL "")
            set (doc_tgt_name "doc_cc_mqttsn_${CC_MQTTSN_CLIENT_CUSTOM_NAME}_client")
        endif ()           

        add_custom_target (${doc_tgt_name}
                COMMAND ${CMAKE_COMMAND} -E copy_if_different 
                    ${CMAKE_CURRENT_SOURCE_DIR}/doxygen/main.dox 
                    ${CMAKE_CURRENT_SOURCE_DIR}/include/cc_mqttsn_client/common.h
                    "${header_output}" 
                    ${interface_doc_dir}/ 
                COMMAND ${DOXYGEN_EXECUTABLE} ${output_file}
                WORKING_DIRECTORY ${interface_doc_dir})
                
        add_dependencies(${doc_tgt_name} ${header_tgt_name})           
    endif ()    
endfunction()

######################################################################

find_package(cc_mqttsn REQUIRED)
find_package (Doxygen)

install (
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/cc_mqttsn_client
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

include(CMakePackageConfigHelpers)

if (CC_MQTTSN_CLIENT_DEFAULT_LIB) 
    gen_lib_mqttsn_client("")
endif ()

if (NOT "${CC_MQTTSN_CUSTOM_CLIENT_CONFIG_FILES}" STREQUAL "")
    foreach (custom_config ${CC_MQTTSN_CUSTOM_CLIENT_CONFIG_FILES})
        gen_lib_mqttsn_client("${custom_config}")
    endforeach ()
endif ()

#add_subdirectory(test)
