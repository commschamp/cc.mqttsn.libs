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

## Dependencies
The messages of MQTT-SN protocol are defined using 
[COMMS library](https://github.com/arobenko/comms_champion#comms-library) from 
[comms_champion](https://github.com/arobenko/comms_champion) project. The MQTT
messages that the **gateway** library uses are defined in 
[mqtt](https://github.com/arobenko/mqtt) project. Both of them are external
projects. It is possible to compile and install them separately and provide
proper paths using **CC_MAIN_INSTALL_DIR** and **CC_MQTT_INSTALL_DIR** options
(see description below). If not provided, their latest releases will be checked 
out and compiled as part of the build process. However, the result of such 
compilation won't be installed together with the artefacts of this project. To
install the produced artefacts of 
[comms_champion](https://github.com/arobenko/comms_champion) and 
[mqtt](https://github.com/arobenko/mqtt) projects use 
**CC_MQTTSN_FULL_SOLUTION** option (also described below).

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
possible to compile other variants of the client library (see 
**CC_MQTTSN_CUSTOM_CLIENT_CONFIG_FILES** option described below). 
Default value is **ON**.

- **CC_MQTTSN_CUSTOM_CLIENT_CONFIG_FILES**=list - Provide list of CMake configuration 
files containing settings for various custom builds of the client library. See
[custom_client_build.md](custom_client_build.md) for details. **NOTE**, that
*list* value is a list of relative or absolute file paths, 
and the elements of the list in CMake are
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
[comms_champion](https://github.com/arobenko/comms_champion) project build. 
If its produced artefacts can be found inside path specified by
**CC_MQTTSN_INSTALL_DIR**, then usage of this variables is unnecessary.

- **CC_MQTT_INSTALL_DIR**=dir - Specify installation path of the 
[mqtt](https://github.com/arobenko/mqtt) project build. The gateway library
uses definition of the MQTT messages from this project. If its produced artefacts can be found inside path specified by
**CC_MQTTSN_INSTALL_DIR**, then usage of this variables is unnecessary.

- **CC_MQTTSN_FULL_SOLUTION**=ON/OFF - Enable/Disable build of 
[comms_champion](https://github.com/arobenko/comms_champion) and
[mqtt](https://github.com/arobenko/mqtt) projects and install the produced
artefacts into the installation directory specified by **CC_MQTTSN_INSTALL_DIR**
variable.

- **CC_MQTTSN_QT_DIR**=dir - Directory of QT5 installation. Can be used to 
provide path to QT5 libraries if differs from system default installation path. If not
provided and QT5 cannot be found on the system, the applications that use QT5 
framework won't be built.

## Configuration Examples

- Build MQTT-SN client and gateway libraries, as well as available client/gateway
applications:
```
$> cmake -DCMAKE_BUILD_TYPE=Release ..
```
- Build only two custom MQTT-SN clients (see [custom_client_build.md](custom_client_build.md)
for details on custom build configuration)
```
$> cmake -DCMAKE_BUILD_TYPE=Release -DCC_MQTTSN_CLIENT_DEFAULT_LIB=OFF \
    -DCC_MQTTSN_BUILD_GATEWAY=OFF \
    -DCC_MQTTSN_CUSTOM_CLIENT_CONFIG_FILES=config1.cmake\;config2.cmake ..
```
- Build only gateway library and applications
```
$> cmake -DCMAKE_BUILD_TYPE=Release CC_MQTTSN_CLIENT_DEFAULT_LIB=OFF ..
```
- Build and install everything while having external builds of 
[comms_champion](https://github.com/arobenko/comms_champion) and
[mqtt](https://github.com/arobenko/mqtt)
```
$> cmake -DCMAKE_BUILD_TYPE=Release -DCC_MQTTSN_BUILD_PLUGINS=ON \
    -DCC_MAIN_INSTALL_DIR=/path/to/comms_champion/install \
    -DCC_MQTT_INSTALL_DIR=/path/to/mqtt/install ..
```
- Build and install everything into the same installation directory as
both [comms_champion](https://github.com/arobenko/comms_champion) and
[mqtt](https://github.com/arobenko/mqtt)
```
$> cmake -DCMAKE_BUILD_TYPE=Release -DCC_MQTTSN_BUILD_PLUGINS=ON \
    -DCC_MQTTSN_INSTALL_DIR=/path/to/comms_champion/install  ..
```
- Build and install full solution, i.e. [comms_champion](https://github.com/arobenko/comms_champion),
[mqtt](https://github.com/arobenko/mqtt), and this project.
```
$> cmake -DCMAKE_BUILD_TYPE=Release -DCC_MQTTSN_BUILD_PLUGINS=ON \
    -DCC_MQTTSN_FULL_SOLUTION=ON  ..
```

