macro (mqttsn_append_option list opt val)
    if (NOT "${val}" STREQUAL "")
        if (NOT "${${list}}" STREQUAL "")
            set (${list} "${${list}},")
        endif ()
        
        set (${list} "${${list}}${opt}<${val}>")
    endif () 
endmacro ()

######################################################################

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if ((CC_MQTTSN_CLIENT_DEFAULT_LIB) AND
    ("${CC_MQTTSN_DEFAULT_CLIENT_CONFIG_FILE}" STREQUAL ""))
    gen_lib_mqttsn_client("${DEFAULT_CLIENT_NAME}" "" TRUE "")
endif ()    

if (NOT "${CC_MQTTSN_DEFAULT_CLIENT_CONFIG_FILE}" STREQUAL "")
    set (CC_MQTTSN_CUSTOM_CLIENT_CONFIG_FILES
        ${CC_MQTTSN_DEFAULT_CLIENT_CONFIG_FILE}
        ${CC_MQTTSN_CUSTOM_CLIENT_CONFIG_FILES})
endif ()

foreach (f ${CC_MQTTSN_CUSTOM_CLIENT_CONFIG_FILES})
    if (NOT EXISTS "${f}")
        message (WARNING "File ${f} does not exist")
        continue()
    endif ()
    
    include (${f})  
    
    if ("${f}" STREQUAL "${CC_MQTTSN_DEFAULT_CLIENT_CONFIG_FILE}")
        set (CC_MQTTSN_CUSTOM_CLIENT_NAME "${DEFAULT_CLIENT_NAME}")
    elseif ("${CC_MQTTSN_CUSTOM_CLIENT_NAME}" STREQUAL "")
        message (WARNING "File ${f} does not specify client name!")
        continue ()
    endif ()
    
    set (mqttsn_library_opts)
    
    mqttsn_append_option (
        mqttsn_library_opts 
        "cc_mqttsn_client::option::ClientIdStaticStorageSize"
        "${CC_MQTTSN_CUSTOM_CLIENT_ID_STATIC_STORAGE_SIZE}"
    )
    
    mqttsn_append_option (
        mqttsn_library_opts 
        "cc_mqttsn_client::option::GwAddStaticStorageSize"
        "${CC_MQTTSN_CUSTOM_CLIENT_GW_ADDR_STATIC_STORAGE_SIZE}"
    )
    
    mqttsn_append_option (
        mqttsn_library_opts 
        "cc_mqttsn_client::option::TopicNameStaticStorageSize"
        "${CC_MQTTSN_CUSTOM_CLIENT_TOPIC_NAME_STATIC_STORAGE_SIZE}"
    )
    
    mqttsn_append_option (
        mqttsn_library_opts 
        "cc_mqttsn_client::option::MessageDataStaticStorageSize"
        "${CC_MQTTSN_CUSTOM_CLIENT_MSG_DATA_STATIC_STORAGE_SIZE}"
    )
    
    mqttsn_append_option (
        mqttsn_library_opts 
        "cc_mqttsn_client::option::ClientsAllocLimit"
        "${CC_MQTTSN_CUSTOM_CLIENT_ALLOC_LIMIT}"
    )
    
    mqttsn_append_option (
        mqttsn_library_opts 
        "cc_mqttsn_client::option::TrackedGatewaysLimit"
        "${CC_MQTTSN_CUSTOM_CLIENT_TRACKED_GW_LIMIT}"
    )
    
    mqttsn_append_option (
        mqttsn_library_opts 
        "cc_mqttsn_client::option::RegisteredTopicsLimit"
        "${CC_MQTTSN_CUSTOM_CLIENT_REGISTERED_TOPICS_LIMIT}"
    )

    set (extra_flags)    
    if (CMAKE_COMPILER_IS_GNUCC AND CC_MQTTSN_CUSTOM_CLIENT_NO_STDLIB)
        set (extra_flags "-nostdlib")
    endif ()
    
    
    gen_lib_mqttsn_client("${CC_MQTTSN_CUSTOM_CLIENT_NAME}" "${mqttsn_library_opts}" TRUE "${extra_flags}")
    
    if (CC_MQTTSN_CUSTOM_CLIENT_NO_STDLIB)
        target_compile_definitions("cc_mqttsn_${CC_MQTTSN_CUSTOM_CLIENT_NAME}_client" PRIVATE "-DNOSTDLIB")
    endif ()
endforeach()
