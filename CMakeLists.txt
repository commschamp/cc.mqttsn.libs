cmake_minimum_required (VERSION 3.1)
project ("mqttsn")

option (CC_MQTTSN_NO_WARN_AS_ERR "Do NOT treat warning as error" OFF)
option (CC_MQTTSN_CLIENT_DEFAULT_LIB "Build and install default variant of MQTT-SN Client library" ON)
option (CC_MQTTSN_BUILD_GATEWAY "Build and install MQTT-SN client library(ies) and applications" ON)
option (CC_MQTTSN_NO_UNIT_TESTS "Disable unittests." OFF)
option (CC_MQTTSN_NO_VALGRIND "Disable valgrind in unittests." OFF)
option (CC_MQTTSN_EXTERNALS_UPDATE_DISCONNECTED "Allow skip of external projects update." OFF)
option (CC_MQTTSN_USE_CCACHE "Use ccache on unix system" ON)

# Extra variables
# CC_MAIN_INSTALL_DIR - Path to external installation directory of comms_champion project.
# CC_MQTTSN_GENERATED_INSTALL_DIR - Path to external installation directory of cc.mqttsn.generated project.
# CC_MQTT311_GENERATED_INSTALL_DIR - Path to external installation directory of cc.mqtt311.generated project.
# CC_MQTTSN_CUSTOM_CLIENT_CONFIG_FILES - List of custom client configuration files
# CC_MQTTSN_DEFAULT_CLIENT_CONFIG_FILE - Custom congiruation of the default client.
# CC_MQTTSN_QT_DIR - Path to external Qt5 installation directory

# CC_COMMS_CHAMPION_REPO - Repository of comms_champion project. Defaults to https://github.com/commschamp/comms_champion.git
# CC_COMMS_CHAMPION_TAG - Tag to use for external comms_champion project. Can be empty.
# CC_COMMS_CHAMPION_CMAKE_ARGS - Extra CMake arguments  pass to external comms_champion project

# CC_MQTTSN_PROTOCOL_REPO - Repository of cc.mqttsn.generated project. Defaults to https://github.com/commschamp/cc.mqttsn.generated.git
# CC_MQTTSN_PROTOCOL_TAG - Tag to use for external cc.mqttsn.generated project. Can be empty.
# CC_MQTTSN_PROTOCOL_CMAKE_ARGS - Extra CMake arguments  pass to external cc.mqttsn.generated project

# CC_MQTT311_PROTOCOL_REPO - Repository of cc.mqtt311.generated project. Defaults to https://github.com/commschamp/cc.mqtt311.generated.git
# CC_MQTT311_PROTOCOL_TAG - Tag to use for external cc.mqttsn.generated project. Can be empty.
# CC_MQTT311_PROTOCOL_CMAKE_ARGS - Extra CMake arguments  pass to external cc.mqtt311.generated project


# Updating default settings
if (NOT CMAKE_CXX_STANDARD)
    set (CMAKE_CXX_STANDARD 11)
endif()

if ("${CC_COMMS_CHAMPION_REPO}" STREQUAL "")
    set (CC_COMMS_CHAMPION_REPO "https://github.com/commschamp/comms_champion.git")
endif ()

if ("${CC_COMMS_CHAMPION_TAG}" STREQUAL "")
    set (CC_COMMS_CHAMPION_TAG "v3.1.1")
endif ()

if ("${CC_MQTTSN_PROTOCOL_REPO}" STREQUAL "")
    set (CC_MQTTSN_PROTOCOL_REPO "https://github.com/commschamp/cc.mqttsn.generated.git")
endif ()

if ("${CC_MQTTSN_PROTOCOL_TAG}" STREQUAL "")
    set (CC_MQTTSN_PROTOCOL_TAG "v0.19")
endif ()

if ("${CC_MQTT311_PROTOCOL_REPO}" STREQUAL "")
    set (CC_MQTT311_PROTOCOL_REPO "https://github.com/commschamp/cc.mqtt311.generated.git")
endif ()

if ("${CC_MQTT311_PROTOCOL_TAG}" STREQUAL "")
    set (CC_MQTT311_PROTOCOL_TAG "v0.19")
endif ()

##########################################################################

if (CMAKE_TOOLCHAIN_FILE AND EXISTS ${CMAKE_TOOLCHAIN_FILE})
    message(STATUS "Loading toolchain from ${CMAKE_TOOLCHAIN_FILE}")
endif()

set (EXTERNALS_DIR "${PROJECT_SOURCE_DIR}/externals")
set (CXXTEST_EXTERNAL_DIR "${EXTERNALS_DIR}/cxxtest")

if (NOT CC_MAIN_INSTALL_DIR)
    execute_process (
        COMMAND ${CMAKE_COMMAND} -E make_directory "${EXTERNALS_DIR}"
    )

    set (cc_src_dir "${EXTERNALS_DIR}/comms_champion")
    set (cc_bin_dir "${PROJECT_BINARY_DIR}/comms_champion")
    set (CC_MAIN_INSTALL_DIR "${cc_bin_dir}/install")
        
    include (${PROJECT_SOURCE_DIR}/cmake/CC_Prefetch.cmake)
    cc_prefetch(SRC_DIR ${cc_src_dir} TAG ${CC_COMMS_CHAMPION_TAG})
    
    include (${cc_src_dir}/cmake/CC_External.cmake)
    cc_build_during_config(
        SRC_DIR ${cc_src_dir}
        BUILD_DIR ${cc_bin_dir}
        TAG ${CC_COMMS_CHAMPION_TAG}
        CMAKE_ARGS
            -DCC_COMMS_LIB_ONLY=ON -DCC_NO_UNIT_TESTS=ON
            -DCMAKE_INSTALL_PREFIX=${CC_MAIN_INSTALL_DIR}        
    )

endif ()

list (APPEND CMAKE_PREFIX_PATH "${CC_MAIN_INSTALL_DIR}")
find_package(LibComms NO_MODULE)
set (cc_compile_file ${CC_MAIN_INSTALL_DIR}/lib/LibComms/cmake/CC_Compile.cmake)
if (EXISTS ${cc_compile_file})
    set (warn_as_err_opt)
    if (NOT CC_MQTTSN_NO_WARN_AS_ERR)
        set (warn_as_err_opt WARN_AS_ERR)
    endif ()

    set (ccache_opt)
    if (CC_MQTTSN_USE_CCACHE)
        set (ccache_opt USE_CCACHE)
    endif ()
    include (${cc_compile_file})
    cc_compile(${warn_as_err_opt} ${ccache_opt})
endif ()

if (MSVC)
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)
endif ()

while (TRUE)
    if (CC_MQTTSN_NO_UNIT_TESTS)
        message (STATUS "Unittests are disabled")
        break()
    endif ()
    
    if (CMAKE_CROSSCOMPILING)
        message (STATUS "Not building unittests when cross-compiling")
        break()
    endif ()

    include (${CC_MAIN_INSTALL_DIR}/lib/LibComms/cmake/CC_CxxtestFuncs.cmake)
    cc_get_cxxtest(INSTALL_PREFIX ${EXTERNALS_DIR})

    if (NOT CC_MQTTSN_NO_VALGRIND)
        find_program(VALGRIND_EXECUTABLE NAMES "valgrind")
    endif ()    
    
    break()
endwhile ()

set (INSTALL_DIR ${CMAKE_INSTALL_PREFIX})

include(GNUInstallDirs)
set (INSTALL_NAME "CC_Mqttsn")
set (LIB_INSTALL_DIR ${INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR})
set (BIN_INSTALL_DIR ${INSTALL_DIR}/${CMAKE_INSTALL_BINDIR})
set (ETC_INSTALL_DIR ${INSTALL_DIR}/${CMAKE_INSTALL_SYSCONFDIR})
set (INC_INSTALL_DIR ${INSTALL_DIR}/${CMAKE_INSTALL_INCLUDEDIR})
set (CONFIG_INSTALL_DIR ${INSTALL_DIR}/config)
set (PLUGIN_INSTALL_DIR ${INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}/CommsChampion/plugin)
set (DOC_INSTALL_DIR ${INSTALL_DIR}/${CMAKE_INSTALL_DATAROOTDIR}/doc)

set (DEFAULT_CLIENT_LIB_TGT "cc_mqttsn_client")

if (NOT "${CC_MQTTSN_QT_DIR}" STREQUAL "")
    list (APPEND CMAKE_PREFIX_PATH ${CC_MQTTSN_QT_DIR})
endif ()

######################################################################

set (CC_MQTTSN_EXTERNAL_DIR "${EXTERNALS_DIR}/cc.mqttsn.generated")
set (CC_MQTTSN_EXTERNAL_TGT "mqttsn_external")
set (CC_MQTTSN_EXTERNAL_TGT_INCLUDE)

if ("${CC_MQTTSN_GENERATED_INSTALL_DIR}" STREQUAL "")
    execute_process (
        COMMAND ${CMAKE_COMMAND} -E make_directory "${EXTERNALS_DIR}")
        
    include(ExternalProject)
    
    set (cc_mqttsn_generated_tag "${CC_MQTTSN_PROTOCOL_TAG}")
    set (cc_mqttsn_main_dir "${CMAKE_CURRENT_BINARY_DIR}/cc.mqttsn.generated")
    set (cc_mqttsn_src_dir "${CC_MQTTSN_EXTERNAL_DIR}")
    set (cc_mqttsn_bin_dir "${cc_mqttsn_main_dir}/build")
    set (cc_mqttsn_install_dir "${cc_mqttsn_main_dir}/install")
    set (cc_mqttsn_update_disconnected)
    
    if (CC_MQTTSN_EXTERNALS_UPDATE_DISCONNECTED)
        set (cc_mqttsn_update_disconnected UPDATE_DISCONNECTED 1)
    endif ()
    
    
    ExternalProject_Add(
        "${CC_MQTTSN_EXTERNAL_TGT}"
        PREFIX "${cc_mqttsn_bin_dir}"
        STAMP_DIR "${cc_mqttsn_bin_dir}"    
        GIT_REPOSITORY "${CC_MQTTSN_PROTOCOL_REPO}"
        GIT_TAG "${cc_mqttsn_generated_tag}"
        SOURCE_DIR "${cc_mqttsn_src_dir}"
        CMAKE_ARGS 
            -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
            -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} -DCMAKE_GENERATOR=${CMAKE_GENERATOR}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_INSTALL_PREFIX=${cc_mqttsn_install_dir} 
            -DOPT_NO_COMMS=ON -DOPT_BUILD_TEST=OFF -DOPT_BUILD_PLUGIN=OFF
            -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD} 
            ${CC_MQTTSN_PROTOCOL_CMAKE_ARGS}
        BINARY_DIR "${cc_mqttsn_bin_dir}"
        ${cc_mqttsn_update_disconnected}
    )
    set (CC_MQTTSN_EXTERNAL_TGT_INCLUDE "${cc_mqttsn_install_dir}/include") 
else ()
    list (APPEND CMAKE_PREFIX_PATH "${CC_MQTT311_GENERATED_INSTALL_DIR}")
    find_package(mqtt311 NO_MODULE)
    if (NOT TARGET cc::mqtt311)
        message (FATAL_ERROR "Failed to find external mqtt311 library")
    endif ()
endif ()

######################################################################

add_subdirectory(client)
add_subdirectory(gateway)

