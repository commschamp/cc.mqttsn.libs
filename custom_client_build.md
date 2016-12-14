# Custom Client Build
The [BUILD.md](BUILD.md) file described general build process and explained
usage of **CC_MQTTSN_CUSTOM_CLIENT_CONFIG_FILES** option. Such option specifies 
a single custom client build configuration file or a list of such files. These
files get included during the [CMake](https://cmake.org) parsing stage and 
are expected to specify multiple variables, which in turn influence on the
way the client library is built.

This page describes and explains the meaning of these variables.

## Variables

### MQTTSN_CUSTOM_CLIENT_NAME
The **MQTTSN_CUSTOM_CLIENT_NAME** specifies the name of the custom client library.
It will influence the names of the API functions. The **default** client build
(controlled by **CC_MQTTSN_CLIENT_DEFAULT_LIB** option) prefixes all the 
functions with `mqttsn_client_`, while client with custom name will produce 
functions having `mqttsn_<custom_name>_client_` prefix. For example having the
`set (MQTTSN_CUSTOM_CLIENT_NAME "my_name")` statement in configuration file
will produce a library which prefixes all API functions with 
`mqttsn_my_name_client_`.
