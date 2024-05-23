# CMAKE_CONFIG_FILE - input cmake file
# CMAKE_DEFAULT_CONFIG_VARS - input cmake file setting default variables
# PROT_OPTS_HEADER_TEMPL - protocol options template file
# OUT_FILE - output header file

if ((NOT "${CMAKE_CONFIG_FILE}" STREQUAL "") AND (NOT EXISTS "${CMAKE_CONFIG_FILE}"))
    message (FATAL_ERROR "Input file \"${CMAKE_CONFIG_FILE}\" doesn't exist!")
endif ()

if (NOT EXISTS "${CMAKE_DEFAULT_CONFIG_VARS}")
    message (FATAL_ERROR "Input file \"${CMAKE_DEFAULT_CONFIG_VARS}\" doesn't exist!")
endif ()

if (NOT EXISTS "${PROT_OPTS_HEADER_TEMPL}")
    message (FATAL_ERROR "Input file \"${PROT_OPTS_HEADER_TEMPL}\" doesn't exist!")
endif ()

if (NOT "${CMAKE_CONFIG_FILE}" STREQUAL "")
    include (${CMAKE_CONFIG_FILE})
endif ()

file (READ ${PROT_OPTS_HEADER_TEMPL} text)

include (${CMAKE_DEFAULT_CONFIG_VARS} NO_POLICY_SCOPE)

#########################################

macro (set_default_opt name)
    set (${name} "comms::option::EmptyOption")
endmacro()

#########################################

#set_default_opt (FIELD_BIN_DATA)
#set_default_opt (FIELD_PROPERTIES_LIST)
#set_default_opt (FIELD_PROTOCOL_NAME)
#set_default_opt (FIELD_STRING)
#set_default_opt (FIELD_TOPIC)

#set_default_opt (MESSAGE_CONNECT_FIELDS_CLIENT_ID)
#set_default_opt (MESSAGE_CONNECT_FIELDS_USERNAME)
#set_default_opt (MESSAGE_CONNECT_FIELDS_PASSWORD)
#set_default_opt (MESSAGE_CONNECT_FIELDS_WILL_TOPIC)
#set_default_opt (MESSAGE_SUBSCRIBE_FIELDS_LIST)
#set_default_opt (MESSAGE_UNSUBSCRIBE_FIELDS_LIST)

set_default_opt (MAX_PACKET_SIZE)
#set_default_opt (MSG_ALLOC_OPT)

#########################################

# Update options

if (NOT ${CC_MQTTSN_CLIENT_HAS_DYN_MEM_ALLOC})
#    set (FIELD_PROTOCOL_NAME "comms::option::app::FixedSizeStorage<4>")
#    set (MSG_ALLOC_OPT "comms::option::app::InPlaceAllocation")
endif ()

#if (NOT ${CC_MQTTSN_CLIENT_BIN_DATA_FIELD_FIXED_LEN} EQUAL 0)
#    set (FIELD_BIN_DATA "comms::option::app::FixedSizeStorage<${CC_MQTTSN_CLIENT_BIN_DATA_FIELD_FIXED_LEN}>")
#endif ()

#if (NOT ${CC_MQTTSN_CLIENT_STRING_FIELD_FIXED_LEN} EQUAL 0)
#    set (FIELD_STRING "comms::option::app::FixedSizeStorage<${CC_MQTTSN_CLIENT_STRING_FIELD_FIXED_LEN}>")
#endif ()

if (NOT ${CC_MQTTSN_CLIENT_MAX_OUTPUT_PACKET_SIZE} EQUAL 0)
    set (MAX_PACKET_SIZE "comms::option::app::FixedSizeStorage<${CC_MQTTSN_CLIENT_MAX_OUTPUT_PACKET_SIZE}>")
endif ()

#if (NOT ${CC_MQTTSN_CLIENT_CLIENT_ID_FIELD_FIXED_LEN} EQUAL 0)
#    set (MESSAGE_CONNECT_FIELDS_CLIENT_ID "comms::option::app::FixedSizeStorage<${CC_MQTTSN_CLIENT_CLIENT_ID_FIELD_FIXED_LEN}>")
#endif ()

#if (NOT ${CC_MQTTSN_CLIENT_USERNAME_FIELD_FIXED_LEN} EQUAL 0)
#    set (MESSAGE_CONNECT_FIELDS_USERNAME "comms::option::app::FixedSizeStorage<${CC_MQTTSN_CLIENT_USERNAME_FIELD_FIXED_LEN}>")
#endif ()

#if (NOT ${CC_MQTTSN_CLIENT_PASSWORD_FIELD_FIXED_LEN} EQUAL 0)
#    set (MESSAGE_CONNECT_FIELDS_PASSWORD "comms::option::app::FixedSizeStorage<${CC_MQTTSN_CLIENT_PASSWORD_FIELD_FIXED_LEN}>")
#endif ()

#if (NOT ${CC_MQTTSN_CLIENT_TOPIC_FIELD_FIXED_LEN} EQUAL 0)
#    set (FIELD_TOPIC "comms::option::app::FixedSizeStorage<${CC_MQTTSN_CLIENT_TOPIC_FIELD_FIXED_LEN}>")    
#    set (MESSAGE_CONNECT_FIELDS_WILL_TOPIC "comms::option::app::FixedSizeStorage<${CC_MQTTSN_CLIENT_TOPIC_FIELD_FIXED_LEN}>")
#endif ()

#if (NOT ${CC_MQTTSN_CLIENT_ASYNC_SUBS_LIMIT} EQUAL 0)
#    set (MESSAGE_SUBSCRIBE_FIELDS_LIST "comms::option::app::FixedSizeStorage<${CC_MQTTSN_CLIENT_ASYNC_SUBS_LIMIT}>")
#endif ()

#if (NOT ${CC_MQTTSN_CLIENT_ASYNC_UNSUBS_LIMIT} EQUAL 0)
#    set (MESSAGE_UNSUBSCRIBE_FIELDS_LIST "comms::option::app::FixedSizeStorage<${CC_MQTTSN_CLIENT_ASYNC_UNSUBS_LIMIT}>")
#endif ()


#########################################

#replace_in_text (FIELD_BIN_DATA)
#replace_in_text (FIELD_PROPERTIES_LIST)
#replace_in_text (FIELD_PROTOCOL_NAME)
#replace_in_text (FIELD_STRING)
#replace_in_text (FIELD_TOPIC)

#replace_in_text (MAX_PACKET_SIZE)
#replace_in_text (MSG_ALLOC_OPT)

file (WRITE "${OUT_FILE}.tmp" "${text}")

execute_process(
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${OUT_FILE}.tmp" "${OUT_FILE}")    

execute_process(
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${OUT_FILE}.tmp")      



