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
[script/prepare_externals.sh](../script/prepare_externals.sh) for Linux and
[script/prepare_externals.bat](../script/prepare_externals.bat) for Windows)
which can help in preparation of these dependencies. They are also used
in configuration of the [github actions](../.github/workflows/actions_build.yml).  

The provided **applications** use [Boost](https://www.boost.org) libraries to
parse their command line arguments as well as manage their event loop / network
connections. In case the application are compiled and the [Boost](https://www.boost.org) libraries
do not reside in a default system location, use the relevant variables described in the 
[FindBoost](https://cmake.org/cmake/help/latest/module/FindBoost.html)
documentation to help with finding appropriate boost libraries. 

## Choosing C++ Standard
The default and minimal required C++ standard version to build this project is **17**. However it
is possible to increase it using the **CMAKE_CXX_STANDARD** cmake variable.

## Building as Shared Library
By default the library is built as static one. It is possible to build it as a shared library
by using the built-in **BUILD_SHARED_LIBS** cmake option

## Forcing Position Independent Code
When the libraries are built as **shared** ones then the position independent code is enabled
automatically. However, when the libraries built as **static**, but will become part of
some other shared libraries, then forcefully enabling the position independent code may be required.
Use provided **CC_MQTTSN_CLIENT_LIB_FORCE_PIC** and/or **CC_MQTTSN_GATEWAY_LIB_FORCE_PIC** cmake
options to enable it.

Note that it is also possible to force position independent code using global
**CMAKE_POSITION_INDEPENDENT_CODE** variable set to **ON**. In such case all the
application will also be compiled with position independent code.

## Examples of Build and Install
The examples below are Linux/Unix system oriented, i.e. they use **make** utility
to build the "install" target after configuration with **cmake**. For Windows
platforms please remember to use **-G** option to specify the generator and with
later versions of cmake also **-A** option to specify the architecture, such as
**-G "NMake Makefiles"** (required environment setup beforehand) or
**cmake -G "Visual Studio 16 2019" -A x64**.

Please review the examples below and use appropriate option that suites your
needs. Remember to use **-DCMAKE_BUILD_TYPE=Release** option for release
builds.


### Build All Libraries and Applications
```
$> mkdir build && cd build
$> cmake .. -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=/path/to/comms/install\;/path/to/cc.mqtt311.generated/install\;/path/to/cc.mqttsn.generated/install
$> cmake --build . --config Release --target install
```

### Build Only Libraries Without Applications
```
$> mkdir build && cd build
$> cmake .. -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH=/path/to/comms/install\;/path/to/cc.mqtt311.generated/install;/path/to/cc.mqttsn.generated/install \
    -DCC_MQTTSN_CLIENT_APPS=OFF -DCC_MQTTSN_GATEWAY_APPS=OFF
$> cmake --build . --config Release --target install
```

### Build as Shared Libraries
```
$> mkdir build && cd build
$> cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON \
    -DCMAKE_PREFIX_PATH=/path/to/comms/install\;/path/to/cc.mqtt311.generated/install;/path/to/cc.mqttsn.generated/install
$> cmake --build . --config Release --target install
```

### Build Static Libraries With Position Independent Code
```
$> mkdir build && cd build
$> cmake .. -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH=/path/to/comms/install\;/path/to/cc.mqtt311.generated/install;/path/to/cc.mqttsn.generated/install \
    -DCC_MQTTSN_CLIENT_LIB_FORCE_PIC=ON -DCC_MQTTSN_GATEWAY_LIB_FORCE_PIC=ON
$> cmake --build . --config Release --target install
```

### Build Two Custom Client Libraries Without Gateway
See [custom_client_build.md](custom_client_build.md)
for details on custom build configuration
```
$> mkdir build && cd build
$> cmake .. -DCMAKE_BUILD_TYPE=Release -DCC_MQTTSN_CLIENT_DEFAULT_LIB=OFF -DCC_MQTTSN_GATEWAY_LIB=OFF \
    -DCC_MQTTSN_CUSTOM_CLIENT_CONFIG_FILES=config1.cmake\;config2.cmake \
    -DCMAKE_PREFIX_PATH=/path/to/comms/install\;/path/to/cc.mqttsn.generated/install
$> cmake --build . --config Release --target install
```




