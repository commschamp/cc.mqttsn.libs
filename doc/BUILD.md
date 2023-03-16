# How to Build

This project uses [CMake](https://cmake.org) cross-platform build system to
generate required build files native to the platform. Please refer to the 
main [CMakeLists.txt](../CMakeLists.txt) file for the info on available options and 
other variables.

## External Dependencies
The provided libraries depend on several external projects:
- [cc.mqttsn.generated](https://github.com/commschamp/cc.mqttsn.generated) -
  provides definition of the MQTT-SN protocol.
- [cc.mqtt311.generated](https://github.com/commschamp/cc.mqtt311.generated) - 
  provides definition of the MQTT v3.1.1 protocol.
- [comms](https://github.com/commschamp/comms) - 
  provides [COMMS library](https://github.com/commschamp/comms)
  which is used to define the protocols.

These dependencies are expected to be built independenty and access to them provided
via standard **CMAKE_PREFIX_PATH** cmake variable. There are also scripts (
[script/prepare_externals.sh](script/prepare_externals.sh) for Linux and
[script/prepare_externals.bat](script/prepare_externals.bat) for Windows)
which can help in preparation of these dependencies. They are also used
in configuration of the [github actions](.github/workflows/actions_build.yml) and
[appveyor](.appveyor.yml).  


## Choosing C++ Standard
Since CMake v3.1 it became possible to set version of C++ standard by setting
**CMAKE_CXX_STANDARD** variable. If no value of this variable is set in command
line arguments, default value **11** will be assigned to it. In order to use
c++14 standard in compilation, set the variable value to **14**. 

Please **NOTE**, that some older versions of the _clang_ compiler habe problems 
compiling valid c++11 constructs used in this project. 
Hence, the compilation will fail unless the compilation is
configured to use c++14 standard.

## Build and Install Examples

The examples below are Linux/Unix system oriented, i.e. they use **make** utility
to build the "install" target after configuration with **cmake**. 

On Windows
systems with Visual Studio compiler, the CMake utility generates Visual Studio
solution files by default. Build "install" project. It is also possible to 
generate Makefile-s on Windows by providing additional **-G "NMake Makefiles"** option
to **cmake**. In this case use **nmake** utility instead of **make**.

Please review the examples below and use appropriate option that suites your
needs. Remember to add **-DCMAKE_BUILD_TYPE=Release** option for release
builds.

### Build MQTT-SN Client/Gateway Libraries and Applications
```
$> cmake .. -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=/path/to/comms/install\;/path/to/cc.mqttsn.generated/install\;/path/to/cc.mqtt311.generated/install
```

### Build Two Custom Client Libraries
See [custom_client_build.md](custom_client_build.md)
for details on custom build configuration
```
$> cmake .. -DCMAKE_BUILD_TYPE=Release -DCC_MQTTSN_CLIENT_DEFAULT_LIB=OFF \
    -DCC_MQTTSN_BUILD_GATEWAY=OFF \
    -DCC_MQTTSN_CUSTOM_CLIENT_CONFIG_FILES=config1.cmake\;config2.cmake \
    -DCMAKE_PREFIX_PATH=/path/to/comms/install\;/path/to/cc.mqttsn.generated/install
```

### Build Gateway Library and Application(s)
```
$> cmake .. -DCMAKE_BUILD_TYPE=Release -DCC_MQTTSN_CLIENT_DEFAULT_LIB=OFF \
  -DCMAKE_PREFIX_PATH=/path/to/comms/install\;/path/to/cc.mqttsn.generated/install\;/path/to/cc.mqtt311.generated/install
```



