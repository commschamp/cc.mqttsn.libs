# CMAKE_CONFIG_FILE - input cmake file
# CMAKE_DEFAULT_CONFIG_VARS - input cmake file setting default variables
# CONFIG_HEADER_TEMPL - config template file
# OUT_FILE - output header file

if ((NOT "${CMAKE_CONFIG_FILE}" STREQUAL "") AND (NOT EXISTS "${CMAKE_CONFIG_FILE}"))
    message (FATAL_ERROR "Input file \"${CMAKE_CONFIG_FILE}\" doesn't exist!")
endif ()

if (NOT EXISTS "${CMAKE_DEFAULT_CONFIG_VARS}")
    message (FATAL_ERROR "Input file \"${CMAKE_DEFAULT_CONFIG_VARS}\" doesn't exist!")
endif ()

if (NOT EXISTS "${CONFIG_HEADER_TEMPL}")
    message (FATAL_ERROR "Input file \"${CONFIG_HEADER_TEMPL}\" doesn't exist!")
endif ()

if (NOT "${CMAKE_CONFIG_FILE}" STREQUAL "")
    include (${CMAKE_CONFIG_FILE})
endif ()

file (READ ${CONFIG_HEADER_TEMPL} text)

include (${CMAKE_DEFAULT_CONFIG_VARS} NO_POLICY_SCOPE)

#########################################

macro (adjust_bool_value name adjusted_name)
    if (${${name}})
        set (${adjusted_name} "true")
    else ()
        set (${adjusted_name} "false")
    endif ()
endmacro ()

adjust_bool_value ("CC_MQTTSN_CLIENT_HAS_DYN_MEM_ALLOC" "CC_MQTTSN_CLIENT_HAS_DYN_MEM_ALLOC_CPP")
adjust_bool_value ("CC_MQTTSN_CLIENT_HAS_GATEWAY_DISCOVERY" "CC_MQTTSN_CLIENT_HAS_GATEWAY_DISCOVERY_CPP")
adjust_bool_value ("CC_MQTTSN_CLIENT_HAS_WILL" "CC_MQTTSN_CLIENT_HAS_WILL_CPP")
adjust_bool_value ("CC_MQTTSN_CLIENT_HAS_ERROR_LOG" "CC_MQTTSN_CLIENT_HAS_ERROR_LOG_CPP")
adjust_bool_value ("CC_MQTTSN_CLIENT_HAS_TOPIC_FORMAT_VERIFICATION" "CC_MQTTSN_CLIENT_HAS_TOPIC_FORMAT_VERIFICATION_CPP")
adjust_bool_value ("CC_MQTTSN_CLIENT_HAS_SUB_TOPIC_VERIFICATION" "CC_MQTTSN_CLIENT_HAS_SUB_TOPIC_VERIFICATION_CPP")

#########################################

replace_in_text (CC_MQTTSN_CLIENT_HAS_DYN_MEM_ALLOC_CPP)
replace_in_text (CC_MQTTSN_CLIENT_ALLOC_LIMIT)
replace_in_text (CC_MQTTSN_CLIENT_HAS_GATEWAY_DISCOVERY_CPP)
replace_in_text (CC_MQTTSN_CLIENT_GATEWAY_INFOS_MAX_LIMIT)
replace_in_text (CC_MQTTSN_CLIENT_GATEWAY_ADDR_FIXED_LEN)
replace_in_text (CC_MQTTSN_CLIENT_HAS_WILL_CPP)
replace_in_text (CC_MQTTSN_CLIENT_MAX_OUTPUT_PACKET_SIZE)
replace_in_text (CC_MQTTSN_CLIENT_ASYNC_PUBS_LIMIT)
replace_in_text (CC_MQTTSN_CLIENT_ASYNC_SUBS_LIMIT)
replace_in_text (CC_MQTTSN_CLIENT_ASYNC_UNSUBS_LIMIT)
replace_in_text (CC_MQTTSN_CLIENT_HAS_ERROR_LOG_CPP)
replace_in_text (CC_MQTTSN_CLIENT_HAS_TOPIC_FORMAT_VERIFICATION_CPP)
replace_in_text (CC_MQTTSN_CLIENT_HAS_SUB_TOPIC_VERIFICATION_CPP)
replace_in_text (CC_MQTTSN_CLIENT_SUB_FILTERS_LIMIT)
replace_in_text (CC_MQTTSN_CLIENT_IN_REG_TOPICS_LIMIT)
replace_in_text (CC_MQTTSN_CLIENT_OUT_REG_TOPICS_LIMIT)
replace_in_text (CC_MQTTSN_CLIENT_MAX_QOS)

file (WRITE "${OUT_FILE}.tmp" "${text}")

execute_process(
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${OUT_FILE}.tmp" "${OUT_FILE}")    

execute_process(
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${OUT_FILE}.tmp")
