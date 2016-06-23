cmake_minimum_required (VERSION 2.8.11)
project ("mqttsn")

option (MQTTSN_LIB_ONLY "Install only MQTT protocol library, no other applications/plugings are built." OFF)
option (MQTTSN_CC_PROTOCOL_PLUGIN "Build and install protocol plugin for CommsChampion." ON)
option (MQTTSN_CC_PLUGIN_COPY_TO_CC_INSTALL_PATH "Copy pluginS for CommsChampion to the install path of the latter." ON)

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
elseif (MSVC)
    add_definitions( "/wd4503" "/wd4309" "/wd4267" "-D_SCL_SECURE_NO_WARNINGS")
endif ()

set (INSTALL_DIR ${CMAKE_BINARY_DIR}/install)
set (LIB_INSTALL_DIR ${INSTALL_DIR}/lib)
set (BIN_INSTALL_DIR ${INSTALL_DIR}/bin)
set (INC_INSTALL_DIR ${INSTALL_DIR}/include)
set (PLUGIN_INSTALL_DIR ${INSTALL_DIR}/plugin)

install (
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/mqttsn
    DESTINATION ${INC_INSTALL_DIR}
)

if (MQTT_LIB_ONLY)
    return ()
endif ()   

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include)

add_subdirectory(cc_plugin)

