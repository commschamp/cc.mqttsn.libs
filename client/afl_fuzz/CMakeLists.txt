if (NOT CC_MQTTSN_AFL_FUZZ)
    return ()
endif ()


set (SRC_TEMPL ${CMAKE_CURRENT_SOURCE_DIR}/AflFuzz.cpp.templ)
set (TEMPL_PROCESS_SCRIPT ${PROJECT_SOURCE_DIR}/cmake/ProcessTemplate.cmake)
set (AFL_FUZZ_COMMON_LIB "cc_mqttsn_client_afl_fuzz_common")

######################################################################

function (cc_mqttsn_client_afl_fuzz_common_lib)

    set (src
        Generator.cpp
        Logger.cpp
        ProgramOptions.cpp
        main.cpp
    )

    add_library(${AFL_FUZZ_COMMON_LIB} STATIC ${src})
    target_link_libraries(${AFL_FUZZ_COMMON_LIB} PUBLIC cc::cc_mqttsn cc::comms Boost::program_options)
    target_include_directories(${AFL_FUZZ_COMMON_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
endfunction()

######################################################################

function (cc_mqttsn_client_afl_fuzz config_file)
    if (NOT "${config_file}" STREQUAL "")
        include (${config_file})
    endif()

    set (name "${CC_MQTTSN_CLIENT_CUSTOM_NAME}")
    if ("${CC_MQTTSN_CLIENT_CUSTOM_NAME}" STREQUAL "")
        set (dir ${DEFAULT_CLIENT_DIR_NAME})
        set (lib_name "cc_mqttsn_client")
        set (app_name "cc_mqttsn_client_afl_fuzz")
    else ()
        set (dir "${CC_MQTTSN_CLIENT_CUSTOM_NAME}")
        set (name "${name}_")
        set (lib_name "cc_mqttsn_${name}client")
        set (app_name "cc_mqttsn_${name}client_afl_fuzz")
    endif ()
    
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/${dir})    

    set (src_output ${CMAKE_CURRENT_BINARY_DIR}/${dir}/${name}AflFuzz.cpp)

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
    
    set (src_tgt_name "${name}main.cpp.tgt") 
    add_custom_target(
        ${src_tgt_name}
        DEPENDS "${src_output}" ${SRC_TEMPL} ${TEMPL_PROCESS_SCRIPT}
    )

    set (src
        ${src_output}
    )

    add_executable(${app_name} ${src})
    target_link_libraries(${app_name} PRIVATE ${AFL_FUZZ_COMMON_LIB} cc::${lib_name})
    target_include_directories(${app_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

    add_dependencies(${app_name} ${src_tgt_name})
    
    install (
        TARGETS ${app_name}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endfunction()

######################################################################

find_package (Boost REQUIRED COMPONENTS program_options)

cc_mqttsn_client_afl_fuzz_common_lib()

if (CC_MQTTSN_CLIENT_DEFAULT_LIB) 
    cc_mqttsn_client_afl_fuzz("")
endif ()

if (NOT "${CC_MQTTSN_CUSTOM_CLIENT_CONFIG_FILES}" STREQUAL "")
    foreach (custom_config ${CC_MQTTSN_CUSTOM_CLIENT_CONFIG_FILES})
        cc_mqttsn_client_afl_fuzz("${custom_config}")
    endforeach ()
endif ()