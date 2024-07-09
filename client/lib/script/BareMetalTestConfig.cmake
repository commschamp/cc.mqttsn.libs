# Name of the client API
set (CC_MQTTSN_CLIENT_CUSTOM_NAME "bm")

# Exclude dynamic memory allocation
set (CC_MQTTSN_CLIENT_HAS_DYN_MEM_ALLOC FALSE)

# Allow only a single client
set(CC_MQTTSN_CLIENT_ALLOC_LIMIT 1)

# Allow only one gateway
set(CC_MQTTSN_CLIENT_GATEWAY_INFOS_MAX_LIMIT 1)

# Limit the max "gateway address" length
set(CC_MQTTSN_CLIENT_GATEWAY_ADDR_FIXED_LEN 4)

# Limit the max "client id" length
set(CC_MQTTSN_CLIENT_CLIENT_ID_FIELD_FIXED_LEN 20)

# Limit the max "will topic" length
set(CC_MQTTSN_CLIENT_WILL_TOPIC_FIELD_FIXED_LEN 20)

# Limit the max "will data" length
set(CC_MQTTSN_CLIENT_WILL_DATA_FIELD_FIXED_LEN 128)

# Limit the max "topic" length
set(CC_MQTTSN_CLIENT_TOPIC_FIELD_FIXED_LEN 20)

# Limit the max "data" length
set(CC_MQTTSN_CLIENT_DATA_FIELD_FIXED_LEN 128)

# Limit the length of the buffer required to store serialized message
set(CC_MQTTSN_CLIENT_MAX_OUTPUT_PACKET_SIZE 512)

# Limit the amount of outstanding subscribe operations
set(CC_MQTTSN_CLIENT_ASYNC_SUBS_LIMIT 1)

# Limit the amount of outstanding unsubscribe operations
set(CC_MQTTSN_CLIENT_ASYNC_UNSUBS_LIMIT 1)

# Limit the amount of stored subscribed topic filters
set(CC_MQTTSN_CLIENT_SUB_FILTERS_LIMIT 10)

# Limit the amount of input registered topics
set(CC_MQTTSN_CLIENT_IN_REG_TOPICS_LIMIT 20)
