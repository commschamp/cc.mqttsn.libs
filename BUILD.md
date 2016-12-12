# How to Build

This project uses [CMake](https://cmake.org) cross-platform build system to
generate required build files native to the platform.

- Create build directory somewhere and navigate there.

>$> mkdir /some/build/dir && cd /some/build/dir

- Configure and generate Makefiles (or any other build environment) for **Release** build.

>$> cmake -DCMAKE_BUILD_TYPE=Release /path/to/mqtt-sn/sources

- Build and install using generated build system.

>$> make install

After the build is complete, all the binaries, headers, and libraries will reside
in **install** subdirectory of the directory chosen for build (/some/build/dir),
unless the custom installation directory has been provided as an option to CMake
(see description of **CC_MQTTSN_INSTALL_DIR** below).

## Available CMake Options

In addition to built-in options/variables of CMake, such as **CMAKE_BUILD_TYPE** or
**CMAKE_TOOLCHAIN_FILE**, the following ones can be used:

- **CC_MQTTSN_NO_WARN_AS_ERR**=ON/OFF - By default, all warnings are treated as
errors. Enable this option in case the compiler generates warning and fails the
compilation. Please open the issue when such scenario occurs. Default value is 
**OFF**.

- **CC_MQTTSN_CLIENT_DEFAULT_LIB**=ON/OFF - Enable/Disable build of MQTT-SN
client library with **default** build options as well as available "publish" /
"subscribe" client applications. By default, the MQTT-SN client library
uses types like [std::string](http://en.cppreference.com/w/cpp/string/basic_string)
and [std::vector](http://en.cppreference.com/w/cpp/container/vector), which
may be problematic for bare-metal applications. It is 
possible to compile other variants of the client library (see next option
described). Default value is **ON**.

- **CC_MQTTSN_CUSTOM_CLIENT_CONFIG_FILES**=list - Provide list of files containing
configurations for various custom builds of the client library. See
[custom_client_build.md](custom_client_build.md) for details. **NOTE**, that
*list* value is a list of file paths, and the elements of the list in CMake are
semicolon (**;**) separated. However, depending on the shell environment there 
may be a need to escape the semicolon character with backslash (**\**).

- **CC_MQTTSN_BUILD_GATEWAY**=ON/OFF - Enable/Disable build of MQTT-SN gateway
library and available gateway applications. Default value is **ON**.

- **CC_MQTTSN_BUILD_PLUGINS**=ON/OFF - Enable/Disable build of plugins for
[CommsChampion Tools](https://github.com/arobenko/comms_champion#commschampion-tools).
The plugins are needed to view and monitor traffic of MQTT-SN messages or 
custom messages that use MQTT-SN for transport. Default value is **OFF**.

- **CC_MQTTSN_INSTALL_DIR**=dir - Provide custom installation directory. If
not provided defaults to **install** subdirectory of the directory used for
build.

- **CC_MAIN_INSTALL_DIR**=dir - Specify installation path of the 
[comms_champion](https://github.com/arobenko/comms_champion) build. 
The MQTT-SN protocol messages are defined using
[COMMS library](https://github.com/arobenko/comms_champion#comms-library) from 
[comms_champion](https://github.com/arobenko/comms_champion) project. The usage
of this variable may be omitted, if [comms_champion](https://github.com/arobenko/comms_champion)
and this project share the same installation path, i.e. the 
[comms_champion](https://github.com/arobenko/comms_champion) has already been
built and installed into **CC_MQTTSN_INSTALL_DIR**.

- **CC_MQTT_INSTALL_DIR**=dir - Specify installation path of the 
[mqtt](https://github.com/arobenko/mqtt) project build. The gateway library
uses definition of the MQTT messages from this project. The usage of this
variable may be omitted if either gateway is not built (see 
description of **CC_MQTTSN_BUILD_GATEWAY**) or the appropriate headers can be
found in **CC_MQTTSN_INSTALL_DIR**, i.e. both [mqtt](https://github.com/arobenko/mqtt) 
and this projects are installed in the same location and 
[mqtt](https://github.com/arobenko/mqtt) has already been built and installed.

- **CC_MQTTSN_FULL_SOLUTION**=ON/OFF - As was mentioned in the description of
**CC_MAIN_INSTALL_DIR** and **CC_MQTT_INSTALL_DIR** this project uses headers
from other ones, which must already been built and installed. This option allows
build of the full solution in one go, i.e. it will perform independent checkout of 
[comms_champion](https://github.com/arobenko/comms_champion) and 
[mqtt](https://github.com/arobenko/mqtt), build and install them to 
**CC_MQTTSN_INSTALL_DIR** prior to attempting build of the MQTT-SN client/gateway/
plugins sources.

- **CC_MQTTSN_QT_DIR**=dir - Directory of QT5 installation. Can be used to 
provide path to QT5 if differs from system default installation path. If not
provided and QT5 cannot be found on the system, the applications that use QT5 
framework won't be built.

## Configuration Examples

- Build MQTT-SN client and gateway libraries, as well as available client/gateway
applications:
```
$> cmake -DCMAKE_BUILD_TYPE=Release ..
```
- Build custom MQTT-SN client only (see [custom_client_build.md](custom_client_build.md)
for details on custom build configuration)
```
$> cmake -DCMAKE_BUILD_TYPE=Release -DCC_MQTTSN_CLIENT_DEFAULT_LIB=OFF \
        CC_MQTTSN_BUILD_GATEWAY=OFF CC_MQTTSN_CUSTOM_CLIENT_CONFIG_FILES=config.cmake ..
```

