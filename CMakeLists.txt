cmake_minimum_required (VERSION 2.8.11)
project ("mqttsn")

if (CMAKE_TOOLCHAIN_FILE AND EXISTS ${CMAKE_TOOLCHAIN_FILE})
    message(STATUS "Loading toolchain from ${CMAKE_TOOLCHAIN_FILE}")
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

option (CC_MQTTSN_CLIENT_DEFAULT_LIB "Build and install default variant of MQTT-SN Client library" ON)
option (CC_MQTTSN_FULL_SOLUTION "Build and install full solution, including CommsChampion sources." OFF)
option (CC_NO_GATEWAY_CROSSCOMPILE "When cross compiling, disable build of gateway library and apps." ON)

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
if (NOT "${CC_MQTTSN_INSTALL_DIR}" STREQUAL "")
    set (INSTALL_DIR "${CC_MQTTSN_INSTALL_DIR}")
endif ()

set (LIB_INSTALL_DIR ${INSTALL_DIR}/lib)
set (BIN_INSTALL_DIR ${INSTALL_DIR}/bin)
set (ETC_INSTALL_DIR ${INSTALL_DIR}/etc)
set (DOC_INSTALL_DIR ${INSTALL_DIR}/doc)
set (INC_INSTALL_DIR ${INSTALL_DIR}/include)
set (PLUGIN_INSTALL_DIR ${INSTALL_DIR}/plugin)


set (DEFAULT_CLIENT_LIB_TGT "cc_mqttsn_client")

if (NOT "${CC_MQTTSN_QT_DIR}" STREQUAL "")
    list (APPEND CMAKE_PREFIX_PATH ${CC_MQTTSN_QT_DIR})
endif ()

set (CC_EXTERNAL_TGT "comms_champion_external")
set (CC_MQTT_EXTERNAL_TGT "mqtt_external")
if (CC_MQTTSN_FULL_SOLUTION)
    include(ExternalProject)
    
    set (cc_branch "master")
    set (cc_main_dir "${CMAKE_BINARY_DIR}/comms_champion")
    set (cc_src_dir "${cc_main_dir}/src")
    set (cc_bin_dir "${cc_main_dir}/build")
    set (cc_mqtt_main_dir "${CMAKE_BINARY_DIR}/mqtt")
    set (cc_mqtt_src_dir "${cc_mqtt_main_dir}/src")
    set (cc_mqtt_bin_dir "${cc_mqtt_main_dir}/build")
    
    if (NOT "${CC_MQTTSN_QT_DIR}" STREQUAL "")
        set (extra_cc_args "-DCC_QT_DIR=${CC_MQTTSN_QT_DIR}")
        set (extra_cc_mqtt_args "-DCC_QT_DIR=${CC_MQTTSN_QT_DIR}")
    endif ()

    ExternalProject_Add(
        "${CC_EXTERNAL_TGT}"
        PREFIX "${cc_bin_dir}"
        STAMP_DIR "${cc_bin_dir}"    
        GIT_REPOSITORY "https://github.com/arobenko/comms_champion.git"
        GIT_TAG "${cc_branch}"
        SOURCE_DIR "${cc_src_dir}"
        CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCC_INSTALL_DIR=${INSTALL_DIR} ${extra_cc_args}
        BINARY_DIR "${cc_bin_dir}"
    )
    
    ExternalProject_Add(
        "${CC_MQTT_EXTERNAL_TGT}"
        PREFIX "${cc_mqtt_bin_dir}"
        STAMP_DIR "${cc_mqtt_bin_dir}"    
        GIT_REPOSITORY "https://github.com/arobenko/mqtt.git"
        GIT_TAG "${cc_branch}"
        SOURCE_DIR "${cc_mqtt_src_dir}"
        CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCC_MQTT_INSTALL_DIR=${INSTALL_DIR} -DCC_MQTT_LIB_ONLY=ON ${extra_cc_mqtt_args}
        BINARY_DIR "${cc_mqtt_bin_dir}"
    )
    
    add_dependencies(${CC_MQTT_EXTERNAL_TGT} ${CC_EXTERNAL_TGT})
        
    include_directories("${INSTALL_DIR}/include")
    link_directories("${INSTALL_DIR}/lib")
    set (CC_PLUGIN_LIBRARIES "comms_champion")
    set (CC_COMMS_FOUND TRUE)
    set (CC_COMMS_CHAMPION_FOUND TRUE)
    set (CC_MQTT_FOUND TRUE)
else ()
    if (EXISTS "${INSTALL_DIR}/cmake/CommsChampionConfig.cmake")
        list (APPEND CMAKE_PREFIX_PATH "${INSTALL_DIR}/cmake")
    endif ()
    
    if (NOT "${CC_MAIN_INSTALL_DIR}" STREQUAL "")
        list (APPEND CMAKE_PREFIX_PATH "${CC_MAIN_INSTALL_DIR}/cmake")
    endif ()
    
    if (NOT "${CC_MQTT_INSTALL_DIR}" STREQUAL "")
        list (APPEND CMAKE_PREFIX_PATH "${CC_MQTT_INSTALL_DIR}/cmake")
    endif ()
    
    find_package(CommsChampion)
    
    if (CC_COMMS_CHAMPION_FOUND)
        include_directories(${CC_INCLUDE_DIRS})
        link_directories(${CC_PLUGIN_LIBRARY_DIRS})
    endif ()
    
    find_package(CC_Mqtt)
    
    if (CC_MQTT_FOUND)
        include_directories(${CC_MQTT_INCLUDE_DIRS})
    endif ()
endif ()

add_subdirectory(protocol)
add_subdirectory(client)
add_subdirectory(gateway)
add_subdirectory(cc_plugin)
