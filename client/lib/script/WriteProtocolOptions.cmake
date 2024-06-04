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

set_default_opt (FIELD_GW_ADD)

set_default_opt (MAX_PACKET_SIZE)
set_default_opt (MSG_ALLOC_OPT)

#########################################

# Update options

if (NOT ${CC_MQTTSN_CLIENT_HAS_DYN_MEM_ALLOC})
    set (MSG_ALLOC_OPT "comms::option::app::InPlaceAllocation")
endif ()

if (NOT ${CC_MQTTSN_CLIENT_GATEWAY_ADDR_FIXED_LEN} EQUAL 0)
    set (FIELD_GW_ADD "comms::option::app::FixedSizeStorage<${CC_MQTTSN_CLIENT_GATEWAY_ADDR_FIXED_LEN}>")
elseif (NOT ${CC_MQTTSN_CLIENT_HAS_DYN_MEM_ALLOC})    
    message (FATAL_ERROR "When dynamic memory allocation is disabled, the CC_MQTTSN_CLIENT_GATEWAY_ADDR_FIXED_LEN needs to be set")
endif ()

if (NOT ${CC_MQTTSN_CLIENT_MAX_OUTPUT_PACKET_SIZE} EQUAL 0)
    set (MAX_PACKET_SIZE "comms::option::app::FixedSizeStorage<${CC_MQTTSN_CLIENT_MAX_OUTPUT_PACKET_SIZE}>")
elseif (NOT ${CC_MQTTSN_CLIENT_HAS_DYN_MEM_ALLOC})    
    message (FATAL_ERROR "When dynamic memory allocation is disabled, the CC_MQTTSN_CLIENT_MAX_OUTPUT_PACKET_SIZE needs to be set")
endif ()

#########################################

replace_in_text (FIELD_GW_ADD)

replace_in_text (MAX_PACKET_SIZE)
replace_in_text (MSG_ALLOC_OPT)

file (WRITE "${OUT_FILE}.tmp" "${text}")

execute_process(
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${OUT_FILE}.tmp" "${OUT_FILE}")    

execute_process(
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${OUT_FILE}.tmp")      



