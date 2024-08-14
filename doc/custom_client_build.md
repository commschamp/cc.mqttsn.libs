# Custom Client Build
The [BUILD.md](BUILD.md) file described general build process and explained
usage of **CC_MQTTSN_CUSTOM_CLIENT_CONFIG_FILES** option. Such option specifies 
a single custom client build configuration file or a list of such files. These
files get included during the [CMake](https://cmake.org) parsing stage and 
are expected to specify multiple variables, which in turn influence on the
way the client library is built.

This page describes and explains the meaning of these variables.

## Variables
In general, the client library will use 
[std::string](http://en.cppreference.com/w/cpp/string/basic_string) type to 
hold strings and 
[std::vector](http://en.cppreference.com/w/cpp/container/vector) type to hold
various lists, because there is no known and predefined limit on string length
and/or number of elements in the list. However if such limit is specified, the
library will use [comms::util::StaticString](https://github.com/commschamp/comms/blob/master/include/comms/util/StaticString.h)
and [comms::util::StaticVector](https://github.com/commschamp/comms/blob/master/include/comms/util/StaticVector.h)
from the [COMMS library](https://github.com/commschamp/comms)
instead. These classes don't use exceptions and/or dynamic memory allocation
and can be suitable for bare-metal systems.

### CC_MQTTSN_CUSTOM_CLIENT_NAME
This variable specifies the name of the custom client library.
It will influence the names of the API functions. The **default** client build
(controlled by **CC_MQTTSN_CLIENT_DEFAULT_LIB** option) prefixes all the 
functions with `cc_mqttsn_client_`, while client with custom name will produce 
functions having `cc_mqttsn_<custom_name>_client_` prefix. For example having the
`set (CC_MQTTSN_CUSTOM_CLIENT_NAME "my_name")` statement in configuration file
will produce a library which prefixes all API functions with 
`cc_mqttsn_my_name_client_`.

The **CC_MQTTSN_CUSTOM_CLIENT_NAME** variable is a **must have** one, without it
the custom build of the client library won't be possible.

---
### CC_MQTTSN_CLIENT_HAS_DYN_MEM_ALLOC
Specify whether usage of the dynamic memory allocation is allowed. The value
defaults to **TRUE**. Setting the configuration to **FALSE** will ensure
that all the necessary configuration to avoid dynamic memory allocation has
been provided. In case some of the configuration is missing the compilation will
fail on `static_assert()` invocation with a message of what variable hasn't been
set property.
```
# Disable dynamic memory allocation
set (CC_MQTTSN_CLIENT_HAS_DYN_MEM_ALLOC FALSE)
```

---
### CC_MQTTSN_CLIENT_ALLOC_LIMIT
The client library allows allocation of multiple client managing objects
(**cc_mqttsn_client_alloc()** function). By default the value is **0**,
which means there is no limit on such
allocations. As a result every such object is dynamically allocated.
However, if there is a known limit for number of client managing objects, the
library will be requested to allocate, use **CC_MQTTSN_CLIENT_ALLOC_LIMIT**
variable to specify such limit. If the limit is specified the library will
use static pool for allocation and won't use heap for this purpose.
```
# Only 1 client object will be allocated
set(CC_MQTTSN_CLIENT_ALLOC_LIMIT 1)
```
Having **CC_MQTTSN_CLIENT_HAS_DYN_MEM_ALLOC** set to **FALSE** requires setting
of the **CC_MQTTSN_CLIENT_ALLOC_LIMIT** to a non-**0** value.

---
### CC_MQTTSN_CLIENT_HAS_GATEWAY_DISCOVERY
The client library implements support for gateway discovery. When the
**CC_MQTTSN_CLIENT_HAS_GATEWAY_DISCOVERY** variable is set to **TRUE** (default)
the functionality is enabled and the library allows runtime gateway discovery
control via the API. When the **CC_MQTTSN_CLIENT_HAS_GATEWAY_DISCOVERY** variable is
set to **FALSE** the relevant code is removed by the compiler resulting in smaller
code size and relevant API being stubbed.

```
# Disable gateway discovery
set(CC_MQTTSN_CLIENT_HAS_GATEWAY_DISCOVERY FALSE)
```

---
### CC_MQTTSN_CLIENT_GATEWAY_INFOS_MAX_LIMIT
When gateway discovery is enabled (**CC_MQTTSN_CLIENT_HAS_GATEWAY_DISCOVERY** is set to **TRUE**)
the client library maintains the list of discovered gateways it its internal data structures.
When the **CC_MQTTSN_CLIENT_GATEWAY_INFOS_MAX_LIMIT** variable is set to **0** (default), it means
that there is no limit to the amount of stored gateway infos and as the result
`std::vector<...>` is used to store them in memory.
When the **CC_MQTTSN_CLIENT_GATEWAY_INFOS_MAX_LIMIT**
variable is set to a non-**0** value the
[comms::util::StaticVector](https://github.com/commschamp/comms/blob/master/include/comms/util/StaticVector.h)
is used instead. It can be useful for bare-metal embedded systems without heap.

```
# Allow up to 5 gateways
set(CC_MQTTSN_CLIENT_GATEWAY_INFOS_MAX_LIMIT 5)
```

Having **CC_MQTTSN_CLIENT_HAS_DYN_MEM_ALLOC** set to **FALSE** and
**CC_MQTTSN_CLIENT_HAS_GATEWAY_DISCOVERY** set to **TRUE** requires setting
of the **CC_MQTTSN_CLIENT_GATEWAY_INFOS_MAX_LIMIT** to a non-**0** value.

---
### CC_MQTTSN_CLIENT_GATEWAY_ADDR_FIXED_LEN
When gateway discovery is enabled (**CC_MQTTSN_CLIENT_HAS_GATEWAY_DISCOVERY** is set to **TRUE**)
the client library maintains the list of discovered gateways with their respective
node addresses it its internal data structures.
When the **CC_MQTTSN_CLIENT_GATEWAY_ADDR_FIXED_LEN** variable is set to **0** (default), it means
that there is no limit to the length of the stored gateway address and as the result
`std::vector<...>` is used to store it in memory.
When the **CC_MQTTSN_CLIENT_GATEWAY_ADDR_FIXED_LEN**
variable is set to a non-**0** value the
[comms::util::StaticVector](https://github.com/commschamp/comms/blob/master/include/comms/util/StaticVector.h)
is used instead. It can be useful for bare-metal embedded systems without heap.

```
# Allow up to 4 bytes per gateway address
set(CC_MQTTSN_CLIENT_GATEWAY_ADDR_FIXED_LEN 4)
```

Having **CC_MQTTSN_CLIENT_HAS_DYN_MEM_ALLOC** set to **FALSE** and
**CC_MQTTSN_CLIENT_HAS_GATEWAY_DISCOVERY** set to **TRUE** requires setting
of the **CC_MQTTSN_CLIENT_GATEWAY_ADDR_FIXED_LEN** to a non-**0** value.

---
### CC_MQTTSN_CLIENT_CLIENT_ID_FIELD_FIXED_LEN
To limit the length of the string used to store the "Client ID" information, use
the **CC_MQTTSN_CLIENT_CLIENT_ID_FIELD_FIXED_LEN** variable. When it is set to **0** (default),
it means that there is no limit to the length of the string value and as the result
`std::string` is used to store it in memory.
When the **CC_MQTTSN_CLIENT_CLIENT_ID_FIELD_FIXED_LEN**
variable is set to a non-**0** value the
[comms::util::StaticString](https://github.com/commschamp/comms/blob/master/include/comms/util/StaticString.h)
is used instead. It can be useful for bare-metal embedded systems without heap.

```
# Limit the max "client ID" length
set(CC_MQTTSN_CLIENT_CLIENT_ID_FIELD_FIXED_LEN 50)
```

Having **CC_MQTTSN_CLIENT_HAS_DYN_MEM_ALLOC** set to **FALSE** requires setting
of the **CC_MQTTSN_CLIENT_CLIENT_ID_FIELD_FIXED_LEN** to a non-**0** value.

---
### CC_MQTTSN_CLIENT_HAS_WILL
The client library implements support for will update messages. When the
**CC_MQTTSN_CLIENT_HAS_WILL** variable is set to **TRUE** (default)
the functionality is enabled and the library allows runtime will
control via the API. When the **CC_MQTTSN_CLIENT_HAS_WILL** variable is
set to **FALSE** the relevant code is removed by the compiler resulting in smaller
code size and relevant API being stubbed.

```
# Disable will functionality
set(CC_MQTTSN_CLIENT_HAS_WILL FALSE)
```

---
### CC_MQTTSN_CLIENT_WILL_TOPIC_FIELD_FIXED_LEN
When the will functionality is enabled (**CC_MQTTSN_CLIENT_HAS_WILL** is set to **TRUE**)
and the **CC_MQTTSN_CLIENT_WILL_TOPIC_FIELD_FIXED_LEN** variable is set to **0** (default), it means
that there is no limit to the length of the will topic string and as the result
`std::string` is used to store it in memory.
When the **CC_MQTTSN_CLIENT_WILL_TOPIC_FIELD_FIXED_LEN**
variable is set to a non-**0** value the
[comms::util::StaticString](https://github.com/commschamp/comms/blob/master/include/comms/util/StaticString.h)
is used instead. It can be useful for bare-metal embedded systems without heap.

```
# Allow up to 32 bytes per will topic
set(CC_MQTTSN_CLIENT_WILL_TOPIC_FIELD_FIXED_LEN 32)
```

Having **CC_MQTTSN_CLIENT_HAS_DYN_MEM_ALLOC** set to **FALSE** and
**CC_MQTTSN_CLIENT_HAS_WILL** set to **TRUE** requires setting
of the **CC_MQTTSN_CLIENT_WILL_TOPIC_FIELD_FIXED_LEN** to a non-**0** value.

---
### CC_MQTTSN_CLIENT_WILL_DATA_FIELD_FIXED_LEN
When the will functionality is enabled (**CC_MQTTSN_CLIENT_HAS_WILL** is set to **TRUE**)
and the **CC_MQTTSN_CLIENT_WILL_DATA_FIELD_FIXED_LEN** variable is set to **0** (default), it means
that there is no limit to the length of the will topic string and as the result
`std::vector<...>` is used to store it in memory.
When the **CC_MQTTSN_CLIENT_WILL_DATA_FIELD_FIXED_LEN**
variable is set to a non-**0** value the
[comms::util::StaticVector](https://github.com/commschamp/comms/blob/master/include/comms/util/StaticVector.h)
is used instead. It can be useful for bare-metal embedded systems without heap.

```
# Allow up to 64 bytes per will topic
set(CC_MQTTSN_CLIENT_WILL_DATA_FIELD_FIXED_LEN 64)
```
Having **CC_MQTTSN_CLIENT_HAS_DYN_MEM_ALLOC** set to **FALSE** and
**CC_MQTTSN_CLIENT_HAS_WILL** set to **TRUE** requires setting
of the **CC_MQTTSN_CLIENT_WILL_DATA_FIELD_FIXED_LEN** to a non-**0** value.

---
### CC_MQTTSN_CLIENT_TOPIC_FIELD_FIXED_LEN
When the **CC_MQTTSN_CLIENT_TOPIC_FIELD_FIXED_LEN** variable is set to **0** (default), it means
that there is no limit to the length of the topic string used in various messages and as the result
`std::string` is used to store it in memory.
When the **CC_MQTTSN_CLIENT_TOPIC_FIELD_FIXED_LEN**
variable is set to a non-**0** value the
[comms::util::StaticString](https://github.com/commschamp/comms/blob/master/include/comms/util/StaticString.h)
is used instead. It can be useful for bare-metal embedded systems without heap.

```
# Allow up to 32 bytes per topic
set(CC_MQTTSN_CLIENT_TOPIC_FIELD_FIXED_LEN 32)
```

Having **CC_MQTTSN_CLIENT_HAS_DYN_MEM_ALLOC** set to **FALSE** requires setting
of the **CC_MQTTSN_CLIENT_TOPIC_FIELD_FIXED_LEN** to a non-**0** value.

---
### CC_MQTTSN_CLIENT_DATA_FIELD_FIXED_LEN
When the **CC_MQTTSN_CLIENT_DATA_FIELD_FIXED_LEN** variable is set to **0** (default), it means
that there is no limit to the length of the binary data used in various messages and as the result
`std::vector<...>` is used to store it in memory.
When the **CC_MQTTSN_CLIENT_DATA_FIELD_FIXED_LEN**
variable is set to a non-**0** value the
[comms::util::StaticVector](https://github.com/commschamp/comms/blob/master/include/comms/util/StaticVector.h)
is used instead. It can be useful for bare-metal embedded systems without heap.

```
# Allow up to 128 bytes per binary data
set(CC_MQTTSN_CLIENT_DATA_FIELD_FIXED_LEN 128)
```

Having **CC_MQTTSN_CLIENT_HAS_DYN_MEM_ALLOC** set to **FALSE** requires setting
of the **CC_MQTTSN_CLIENT_DATA_FIELD_FIXED_LEN** to a non-**0** value.

---
### CC_MQTTSN_CLIENT_MAX_OUTPUT_PACKET_SIZE
When serializing the output message the client library needs to allocate an output
buffer. When set to **0** (default), the output buffer type will be dynamic sized
`std::vector<std::uint8_t>`. When the non-**0** value is assigned to the variable, the
[comms::util::StaticVector](https://github.com/commschamp/comms/blob/master/include/comms/util/StaticVector.h)
is used instead. It can be useful for bare-metal embedded systems without heap.

```
# Limit the length of the buffer required to store serialized message
set (CC_MQTTSN_CLIENT_MAX_OUTPUT_PACKET_SIZE 1024)
```

Having **CC_MQTTSN_CLIENT_HAS_DYN_MEM_ALLOC** set to **FALSE** requires setting
of the **CC_MQTTSN_CLIENT_MAX_OUTPUT_PACKET_SIZE** to a non-**0** value.

---
### CC_MQTTSN_CLIENT_ASYNC_PUBS_LIMIT
The library supports issuing multiple "publish" operation in parallel (even though
the protocol requires the messages being sent one at a time). The relevant
information needs to be stored in memory.
When the value of the **CC_MQTTSN_CLIENT_ASYNC_PUBS_LIMIT**
variable is **0** (default), it means that there is no limit for such operations. In such chase the
client library uses `std::vector<...>` to store the relevant operation states in memory.
When **CC_MQTTSN_CLIENT_ASYNC_PUBS_LIMIT** is set to a non-**0** value the
[comms::util::StaticVector](https://github.com/commschamp/comms/blob/master/include/comms/util/StaticVector.h)
is used instead. It can be useful for bare-metal embedded systems without heap.

```
# Limit the amount allowed incomplete publish operations
set (CC_MQTTSN_CLIENT_ASYNC_PUBS_LIMIT 6)
```

Having **CC_MQTTSN_CLIENT_HAS_DYN_MEM_ALLOC** set to **FALSE** requires setting
of the **CC_MQTTSN_CLIENT_ASYNC_PUBS_LIMIT** to a non-**0** value.

---
### CC_MQTTSN_CLIENT_ASYNC_SUBS_LIMIT
The client library allows issuing multiple asynchronous subscription operation. The operation
state needs to be preserved in the memory until
the appropriate acknowledgement message is received from the broker. Setting
the **CC_MQTTSN_CLIENT_ASYNC_SUBS_LIMIT** variable to **0** (default) means there
is no limit to parallel subscription operations and their relative states are
stored using `std::vector<...>` storage type.
When the **CC_MQTTSN_CLIENT_ASYNC_SUBS_LIMIT**
variable is set to a non-**0** value the
[comms::util::StaticVector](https://github.com/commschamp/comms/blob/master/include/comms/util/StaticVector.h)
is used instead. It can be useful for bare-metal embedded systems without heap.

```
# Limit the amount of ongoing (unacknowledged) subscribe operations
set (CC_MQTTSN_CLIENT_ASYNC_SUBS_LIMIT 3)
```

Having **CC_MQTTSN_CLIENT_HAS_DYN_MEM_ALLOC** set to **FALSE** requires setting
of the **CC_MQTTSN_CLIENT_ASYNC_SUBS_LIMIT** to a non-**0** value.

---
### CC_MQTTSN_CLIENT_ASYNC_UNSUBS_LIMIT
The client library allows issuing multiple asynchronous unsubscription operation. The operation
state needs to be preserved in the memory until
the appropriate acknowledgement message is received from the broker. Setting
the **CC_MQTTSN_CLIENT_ASYNC_UNSUBS_LIMIT** variable to **0** (default) means there
is no limit to parallel unsubscription operations and their relative states are
stored using `std::vector<...>` storage type.
When the **CC_MQTTSN_CLIENT_ASYNC_UNSUBS_LIMIT**
variable is set to a non-**0** value the
[comms::util::StaticVector](https://github.com/commschamp/comms/blob/master/include/comms/util/StaticVector.h)
is used instead. It can be useful for bare-metal embedded systems without heap.

```
# Limit the amount of ongoing (unacknowledged) unsubscribe operations
set (CC_MQTTSN_CLIENT_ASYNC_UNSUBS_LIMIT 1)
```

Having **CC_MQTTSN_CLIENT_HAS_DYN_MEM_ALLOC** set to **FALSE** requires setting
of the **CC_MQTTSN_CLIENT_ASYNC_UNSUBS_LIMIT** to a non-**0** value.

---
### CC_MQTTSN_CLIENT_IN_REG_TOPICS_LIMIT
When receiving application messages from the gateway, the latter may issue a
topic registration request. When the **CC_MQTTSN_CLIENT_IN_REG_TOPICS_LIMIT** variable
is set to to **0** (default) the amount of such topic records is unlimited, i.e.
stored using `std::vector<...>` storage type. When the
**CC_MQTTSN_CLIENT_IN_REG_TOPICS_LIMIT** variable is set to non-**0** value the
[comms::util::StaticVector](https://github.com/commschamp/comms/blob/master/include/comms/util/StaticVector.h)
is used instead. It can be useful for bare-metal embedded systems without heap.

```
# Limit the amount of stored incoming registered topic IDs
set (CC_MQTTSN_CLIENT_IN_REG_TOPICS_LIMIT 1)
```

Having **CC_MQTTSN_CLIENT_HAS_DYN_MEM_ALLOC** set to **FALSE** requires setting
of the **CC_MQTTSN_CLIENT_IN_REG_TOPICS_LIMIT** to a non-**0** value.


---
### CC_MQTTSN_CLIENT_OUT_REG_TOPICS_LIMIT
Similar to the **CC_MQTTSN_CLIENT_IN_REG_TOPICS_LIMIT** the
**CC_MQTTSN_CLIENT_OUT_REG_TOPICS_LIMIT** variable controls the storage type
for the topic IDs for the outgoing messages.

```
# Limit the amount of stored outgoing registered topic IDs
set (CC_MQTTSN_CLIENT_OUT_REG_TOPICS_LIMIT 1)
```

Having **CC_MQTTSN_CLIENT_HAS_DYN_MEM_ALLOC** set to **FALSE** requires setting
of the **CC_MQTTSN_CLIENT_OUT_REG_TOPICS_LIMIT** to a non-**0** value.

---
### CC_MQTTSN_CLIENT_HAS_ERROR_LOG
The client library allows reporting various error log messages via callback.
When **CC_MQTTSN_CLIENT_HAS_ERROR_LOG** variable is set to **TRUE** (default) such
error reporting is enabled. Setting the **CC_MQTTSN_CLIENT_HAS_ERROR_LOG** to
**FALSE** removes the error reporting functionality and as the result
reduces the code size and may slightly improve the runtime performance.

```
# Disable the error logging functionality
set (CC_MQTTSN_CLIENT_HAS_ERROR_LOG FALSE)
```

---
### CC_MQTTSN_CLIENT_HAS_TOPIC_FORMAT_VERIFICATION
The client library implements verification of the used topics format to be a
valid one. When the **CC_MQTTSN_CLIENT_HAS_TOPIC_FORMAT_VERIFICATION** variable is
set to **TRUE** (default) the functionality is enabled and the library allows
runtime control of the feature via the API. When the **CC_MQTTSN_CLIENT_HAS_TOPIC_FORMAT_VERIFICATION**
is set to **FALSE** the relevant verification code is removed by the compiler
resulting in smaller code size and improved runtime performance.

```
# Disable the topic format verification functionality
set (CC_MQTTSN_CLIENT_HAS_TOPIC_FORMAT_VERIFICATION FALSE)
```

---
### CC_MQTTSN_CLIENT_HAS_SUB_TOPIC_VERIFICATION
The client library implements tracking of the subscribed topics and discarding
the "rogue" messages from the broker if the received message is not supposed to
be received. When the **CC_MQTTSN_CLIENT_HAS_SUB_TOPIC_VERIFICATION** variable is
set to **TRUE** (default) the functionality is enabled and the library allows
runtime control of the feature via the API. When the **CC_MQTTSN_CLIENT_HAS_SUB_TOPIC_VERIFICATION**
is set to **FALSE** the relevant verification code is removed by the compiler
resulting in smaller code size and improved runtime performance.

```
# Disable the verification that the relevant subscription was performed when the message is reported from the broker
set (CC_MQTTSN_CLIENT_HAS_SUB_TOPIC_VERIFICATION FALSE)
```

---
### CC_MQTTSN_CLIENT_SUB_FILTERS_LIMIT
When the subscription topic verification is enabled
(**CC_MQTTSN_CLIENT_HAS_SUB_TOPIC_VERIFICATION** is set to **TRUE**) the client
library needs to preserve the subscribed topics in the memory. When the
**CC_MQTTSN_CLIENT_SUB_FILTERS_LIMIT** variable is set to **0** (default), it means
that there is no limit to the amount of such topics and as the result
`std::vector<...>` is used to store them in memory.
When the **CC_MQTTSN_CLIENT_SUB_FILTERS_LIMIT**
variable is set to a non-**0** value the
[comms::util::StaticVector](https://github.com/commschamp/comms/blob/master/include/comms/util/StaticVector.h)
is used instead. It can be useful for bare-metal embedded systems without heap.

```
# Limit the amount of topic filters to store when the subscription verification is enabled
#set (CC_MQTTSN_CLIENT_SUB_FILTERS_LIMIT 20)
```

Having **CC_MQTTSN_CLIENT_HAS_DYN_MEM_ALLOC** set to **FALSE** and
**CC_MQTTSN_CLIENT_HAS_TOPIC_FORMAT_VERIFICATION** set to **TRUE** requires setting
of the **CC_MQTTSN_CLIENT_SUB_FILTERS_LIMIT** to a non-**0** value.

---
### CC_MQTTSN_CLIENT_MAX_QOS
By default the library supports all the QoS values (0 to 2). It is possible to
disable support for high QoS values at compile time and as the result reducing
the library's code size. It can be useful for bare-metal embedded system with
a small ROM size.

```
# Support only QoS0 and QoS1 messages
set (CC_MQTTSN_CLIENT_MAX_QOS 1)
```

---
## Example for Bare-Metal Without Heap Configuration
The content of the custom client configuration file, which explicitly specifies
all compile time limits and constants to prevent usage of dynamic
memory allocation and STL types like [std::string](http://en.cppreference.com/w/cpp/string/basic_string)
and [std::vector](http://en.cppreference.com/w/cpp/container/vector), may look
like [this](../client/lib/script/BareMetalTestConfig.cmake):

Setting "bm" as a custom client name results in having a static library called `cc_mqtt311_bm_client`.
All the API functions are defined in `cc_mqtt311_client/bm_client.h` header file:
```c
CC_Mqtt311ClientHandle cc_mqtt311_bm_client_alloc();

void cc_mqtt311_bm_client_free(CC_Mqtt311ClientHandle client);

...
```

