cmake_minimum_required (VERSION 2.8.11)
project ("mqttsn")

if (CMAKE_TOOLCHAIN_FILE AND EXISTS ${CMAKE_TOOLCHAIN_FILE})
    message(STATUS "Loading toolchain from ${CMAKE_TOOLCHAIN_FILE}")
    if(NOT DEFINED CMAKE_CROSSCOMPILING)
        message(FATAL_ERROR "Toolchain file is expected to define CMAKE_SYSTEM_NAME, typically: set (CMAKE_SYSTEM_NAME \"Generic\")")
    endif()
endif()

set (EXTERNALS_DIR "${CMAKE_SOURCE_DIR}/external")

find_package (Git)

if (GIT_FOUND)
    execute_process (
    COMMAND ${GIT_EXECUTABLE} submodule update --init
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif () 

if (NOT CMAKE_CROSSCOMPILING)
    set (CXXTEST_INCLUDE_DIR "${EXTERNALS_DIR}/cxxtest")
    set (CXXTEST_PYTHON_TESTGEN_EXECUTABLE "${CXXTEST_INCLUDE_DIR}/bin/cxxtestgen")
    
    find_package (CxxTest)
    if (CXXTEST_FOUND)
        enable_testing ()
    endif ()
endif ()

if (UNIX AND (NOT CC_MQTTSN_NO_CCACHE))
    find_program(CCACHE_FOUND ccache)
    if(CCACHE_FOUND)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
    endif(CCACHE_FOUND)
endif ()    

option (MQTTSN_INSTALL_PROTOCOL_HEADERS "Install headers for MQTT-SN protocol implementation library. " OFF)
option (MQTTSN_CC_PLUGINS "Build and install pluginS for CommsChampion." ON)
option (MQTTSN_CC_PLUGIN_COPY_TO_CC_INSTALL_PATH "Copy plugins for CommsChampion to the install path of the latter." ON)
option (MQTTSN_CLIENT_DEFAULT_LIB "Build and install default variant of MQTT-SN Client library" ON)

if (CMAKE_COMPILER_IS_GNUCC)
    set (extra_flags_list
        "--std=c++11"
        "-Wall" "-Wextra" "-Werror" "-Wcast-align" "-Wcast-qual" "-Wctor-dtor-privacy"
        "-Wmissing-include-dirs" "-Wnoexcept" "-Wlogical-op" "-Wstrict-null-sentinel"
        "-Woverloaded-virtual" "-Wredundant-decls" "-Wshadow" "-Wundef" "-Wunused"
        "-Wno-unknown-pragmas" "-fdiagnostics-show-option"
    )
    
    string(REPLACE ";" " " extra_flags "${extra_flags_list}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${extra_flags}")
    
    set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined") 
elseif (MSVC)
    add_definitions( "/wd4503" "/wd4309" "/wd4267" "-D_SCL_SECURE_NO_WARNINGS")
endif ()

set (INSTALL_DIR ${CMAKE_BINARY_DIR}/install)
set (LIB_INSTALL_DIR ${INSTALL_DIR}/lib)
set (BIN_INSTALL_DIR ${INSTALL_DIR}/bin)
set (INC_INSTALL_DIR ${INSTALL_DIR}/include)
set (PLUGIN_INSTALL_DIR ${INSTALL_DIR}/plugin)

set (DEFAULT_CLIENT_LIB_TGT "mqttsn_client")

if (NOT "${MQTTSN_QT_DIR}" STREQUAL "")
    list (APPEND CMAKE_PREFIX_PATH ${MQTTSN_QT_DIR})
endif ()

add_subdirectory(protocol)
add_subdirectory(client)
add_subdirectory(gateway)
add_subdirectory(cc_plugin)

