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
library will use [StaticString](https://github.com/commschamp/comms/blob/master/include/comms/util/StaticString.h)
and [StaticVector](https://github.com/commschamp/comms/blob/master/include/comms/util/StaticVector.h)
from [COMMS library](https://github.com/commschamp/comms)
instead. These classes don't use exceptions and/or dynamic memory allocation
and can be suitable for bare-metal systems.

### MQTTSN_CUSTOM_CLIENT_NAME
This variable specifies the name of the custom client library.
It will influence the names of the API functions. The **default** client build
(controlled by **CC_MQTTSN_CLIENT_DEFAULT_LIB** option) prefixes all the 
functions with `cc_mqttsn_client_`, while client with custom name will produce 
functions having `mqttsn_<custom_name>_client_` prefix. For example having the
`set (MQTTSN_CUSTOM_CLIENT_NAME "my_name")` statement in configuration file
will produce a library which prefixes all API functions with 
`mqttsn_my_name_client_`.

The **MQTTSN_CUSTOM_CLIENT_NAME** variable is a **must have** one, without it
the custom build of the client library won't be possible.

### MQTTSN_CUSTOM_CLIENT_ID_STATIC_STORAGE_SIZE
The MQTT-SN needs to store the client ID string. By default it is stored using
[std::string](http://en.cppreference.com/w/cpp/string/basic_string) type. The
**MQTTSN_CUSTOM_CLIENT_ID_STATIC_STORAGE_SIZE** variable can be used to set
the limit to the client ID string known at compile time, as the result the
[StaticString](https://github.com/commschamp/comms/blob/master/include/comms/util/StaticString.h)
type will be used instead.
```
# Allow up to 20 characters in client ID string
set(MQTTSN_CUSTOM_CLIENT_ID_STATIC_STORAGE_SIZE 20)
```

### MQTTSN_CUSTOM_CLIENT_GW_ADDR_STATIC_STORAGE_SIZE
The MQTT-SN protocol defines **SEARCHGW** message, which can contain the 
address of the gateway. The client library doesn't make any assumptions on 
maximum length of the address information. As the result it uses
[std::vector](http://en.cppreference.com/w/cpp/container/vector) type to
store such information. If the limit of the address's length is known at compile
time, use **MQTTSN_CUSTOM_CLIENT_GW_ADDR_STATIC_STORAGE_SIZE** variable to
specify the limit. It will cause 
[StaticVector](https://github.com/commschamp/comms/blob/master/include/comms/util/StaticVector.h)
to be used instead.
```
# The address of GW can be stored within 4 bytes (IP4 for example)
set(MQTTSN_CUSTOM_CLIENT_GW_ADDR_STATIC_STORAGE_SIZE 4)
```

### MQTTSN_CUSTOM_CLIENT_TOPIC_NAME_STATIC_STORAGE_SIZE
Similar to the client id, the maximum length of the topic string is not known, and
[std::string](http://en.cppreference.com/w/cpp/string/basic_string) type is
used as the result. Use **MQTTSN_CUSTOM_CLIENT_TOPIC_NAME_STATIC_STORAGE_SIZE**
option to limit the maximum length and force usage of 
[StaticString](https://github.com/commschamp/comms/blob/master/include/comms/util/StaticString.h)
instead.
```
# The topics contain no more than 64 characters
set(MQTTSN_CUSTOM_CLIENT_TOPIC_NAME_STATIC_STORAGE_SIZE 64)
```

### MQTTSN_CUSTOM_CLIENT_MSG_DATA_STATIC_STORAGE_SIZE
The MQTT-SN protocol defines **PUBLISH** message, which contains binary data
being published. If there is no known limit for the length of such data, the
[std::vector](http://en.cppreference.com/w/cpp/container/vector) type will be
used. However, if there is a limit known at compile time, the 
**MQTTSN_CUSTOM_CLIENT_MSG_DATA_STATIC_STORAGE_SIZE** can be used to specify the
limit and force usage of 
[StaticVector](https://github.com/commschamp/comms/blob/master/include/comms/util/StaticVector.h) 
type instead.
```
# The message data is no more than 128 bytes long
set(MQTTSN_CUSTOM_CLIENT_MSG_DATA_STATIC_STORAGE_SIZE 128)
```

### MQTTSN_CUSTOM_CLIENT_ALLOC_LIMIT
The client library allows allocation of multiple client managing objects
(**cc_mqttsn_client_alloc()** function). By default, there is no limit on such
allocations, and as a result every such object is dynamically allocated.
However, if there is a known limit for number of client managing objects, the
library will be requested to allocate, use ***MQTTSN_CUSTOM_CLIENT_ALLOC_LIMIT**
variable to specify such limit. If the limit is specified the library will 
use static pool for allocation and won't use heap for this purpose.
```
# Only 1 MQTT-SN client will be allocated
set(MQTTSN_CUSTOM_CLIENT_ALLOC_LIMIT 1)
```

### MQTTSN_CUSTOM_CLIENT_TRACKED_GW_LIMIT
The client library monitors and keep information about available gateways. When
the number of possible gateways is not known such information is stored using
[std::vector](http://en.cppreference.com/w/cpp/container/vector) type. However,
if there is a known limit on number of the available gateways on the nenwork,
the client library may be compiled to use 
[StaticVector](https://github.com/commschamp/comms/blob/master/include/comms/util/StaticVector.h) 
instead. Use **MQTTSN_CUSTOM_CLIENT_TRACKED_GW_LIMIT** variable for this purpose.
```
# The library doesn't need to support more than 1 gateway
set(MQTTSN_CUSTOM_CLIENT_TRACKED_GW_LIMIT 1)
```

### MQTTSN_CUSTOM_CLIENT_REGISTERED_TOPICS_LIMIT
The MQTT-SN protocol uses topic IDs in its **PUBLISH** message, and requires
topic strings to be registered with the gateway, which in turn is responsible
to allocate numeric IDs for them. The client object must store the registration
information. By default, there is no known limit on how many topics need to be
registered. As the result the library uses 
[std::vector](http://en.cppreference.com/w/cpp/container/vector) to store all
the required information. However, if there is predefined limit for number of topics
the client object must be able to keep inside, the **MQTTSN_CUSTOM_CLIENT_REGISTERED_TOPICS_LIMIT**
variable can be used to force usage of
[StaticVector](https://github.com/commschamp/comms/blob/master/include/comms/util/StaticVector.h) 
type instead.
```
# No need to support registration of more than 32 topic strings
set(MQTTSN_CUSTOM_CLIENT_REGISTERED_TOPICS_LIMIT 32)
```

### MQTTSN_CUSTOM_CLIENT_NO_STDLIB
Sometimes the bare metal applications are compiled without standard library
(using `-nostdlib` option with gcc compiler). To prevent the client library from
explicitly calling functions provided by the standard library, 
set **MQTTSN_CUSTOM_CLIENT_NO_STDLIB** value to `TRUE`. 
```
# Don't use standard library functions.
set(MQTTSN_CUSTOM_CLIENT_NO_STDLIB TRUE)
```
**NOTE**, that it doesn't prevent the library from using **STL**, and the compiler
may still generate code invoking functions like `memcpy` or `memset`. It will
be a responsibility of the driving code to implement required functionality.

Also **NOTE**, that the library is implemented in **C++** (although it provides
**C** API). It requires manual invocation of the global and static objects'
constructors prior to invocation of the `main()` function. 

It is recommended to read 
[Practical Guide to Bare Metal C++](https://arobenko.github.io/bare_metal_cpp)
free e-book, especially 
[Know Your Compiler Output](https://arobenko.github.io/bare_metal_cpp/#_know_your_compiler_output)
chapter. It will guide the reader through necessary functions that may need
to be implemented to make the bare-metal application, written in C++, work.

## Example for Bare-Metal Without Heap Configuration 
The content of the custom client configuration file, which explicitly specifies
all unknown compile time limits and constants to prevent usage of dynamic 
memory allocation and STL types like [std::string](http://en.cppreference.com/w/cpp/string/basic_string)
and [std::vector](http://en.cppreference.com/w/cpp/container/vector), may look
like this:
```
# Name of the client API
set (MQTTSN_CUSTOM_CLIENT_NAME "bare_metal")

# Use up to 20 characters for client ID
set(MQTTSN_CUSTOM_CLIENT_ID_STATIC_STORAGE_SIZE 20)

# The address of GW can be stored within 2 bytes
set(MQTTSN_CUSTOM_CLIENT_GW_ADDR_STATIC_STORAGE_SIZE 2)

# Support only topics containing no more than 64 characters
set(MQTTSN_CUSTOM_CLIENT_TOPIC_NAME_STATIC_STORAGE_SIZE 64)

# The message data is no more than 128 bytes long
set(MQTTSN_CUSTOM_CLIENT_MSG_DATA_STATIC_STORAGE_SIZE 128)

# The library won't support more than 1 allocated client object
set(MQTTSN_CUSTOM_CLIENT_ALLOC_LIMIT 1)

# The library doesn't need to support more than 1 gateway
set(MQTTSN_CUSTOM_CLIENT_TRACKED_GW_LIMIT 1)

# No need to support registration of more than 8 topic strings
set(MQTTSN_CUSTOM_CLIENT_REGISTERED_TOPICS_LIMIT 8)

# Don't use standard library functions.
set(MQTTSN_CUSTOM_CLIENT_NO_STDLIB TRUE)
```
As the result of such configuration, the static library `cc_mqttsn_bare_metal_client`
will be generated, which will contain functions defined in 
`include/cc_mqttsn_client/bare_metal_client.h" header file:
```c
MqttsnClientHandle mqttsn_bare_metal_client_new();

void mqttsn_bare_metal_client_free(MqttsnClientHandle client);

void mqttsn_bare_metal_client_set_next_tick_program_callback(
    MqttsnClientHandle client,
    MqttsnNextTickProgramFn fn,
    void* data);
    
void mqttsn_bare_metal_client_set_cancel_next_tick_wait_callback(
    MqttsnClientHandle client,
    MqttsnCancelNextTickWaitFn fn,
    void* data);

...
    
```
**NOTE**, that all the functions have **mqttsn_bare_metal_** prefix due to the
fact of setting value of **MQTTSN_CUSTOM_CLIENT_NAME** variable to "bare_metal" string.
