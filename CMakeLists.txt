cmake_minimum_required (VERSION 3.1)
project ("cc_mqttsn")

option (CC_MQTTSN_NO_WARN_AS_ERR "Do NOT treat warning as error" OFF)
option (CC_MQTTSN_CLIENT_DEFAULT_LIB "Build and install default variant of MQTT-SN Client library" ON)
option (CC_MQTTSN_BUILD_GATEWAY "Build and install MQTT-SN client library(ies) and applications" ON)
option (CC_MQTTSN_BUILD_CLIENT_APPS "Build and install client applications" ${CC_MQTTSN_CLIENT_DEFAULT_LIB})
option (CC_MQTTSN_BUILD_GATEWAY_APPS "Build and install gateway applications" ${CC_MQTTSN_BUILD_GATEWAY})
option (CC_MQTTSN_NO_UNIT_TESTS "Disable unittests." OFF)
option (CC_MQTTSN_NO_VALGRIND "Disable valgrind in unittests." OFF)
option (CC_MQTTSN_EXTERNALS_UPDATE_DISCONNECTED "Allow skip of external projects update." OFF)
option (CC_MQTTSN_USE_CCACHE "Use ccache on unix system" ON)

# Extra variables
# CC_MQTTSN_CUSTOM_CLIENT_CONFIG_FILES - List of custom client configuration files
# CC_MQTTSN_DEFAULT_CLIENT_CONFIG_FILE - Custom congiruation of the default client.

# Updating default settings
set(CMAKE_CXX_STANDARD 11 CACHE STRING "The C++ standard to use")

##########################################################################

if (CMAKE_TOOLCHAIN_FILE AND EXISTS ${CMAKE_TOOLCHAIN_FILE})
    message(STATUS "Loading toolchain from ${CMAKE_TOOLCHAIN_FILE}")
endif()

set (EXTERNALS_DIR "${PROJECT_SOURCE_DIR}/externals")
set (CXXTEST_EXTERNAL_DIR "${EXTERNALS_DIR}/cxxtest")

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

    find_package(LibComms REQUIRED)

    include (${LibComms_DIR}/CC_CxxtestFuncs.cmake)
    cc_get_cxxtest(INSTALL_PREFIX ${EXTERNALS_DIR})

    if (NOT CC_MQTTSN_NO_VALGRIND)
        find_program(VALGRIND_EXECUTABLE NAMES "valgrind")
    endif ()    
    
    break()
endwhile ()

set (INSTALL_DIR ${CMAKE_INSTALL_PREFIX})

include(GNUInstallDirs)
set (LIB_INSTALL_DIR ${INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR})
set (BIN_INSTALL_DIR ${INSTALL_DIR}/${CMAKE_INSTALL_BINDIR})
set (ETC_INSTALL_DIR ${INSTALL_DIR}/${CMAKE_INSTALL_SYSCONFDIR})
set (INC_INSTALL_DIR ${INSTALL_DIR}/${CMAKE_INSTALL_INCLUDEDIR})
set (CONFIG_INSTALL_DIR ${INSTALL_DIR}/config)
set (PLUGIN_INSTALL_DIR ${INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}/CommsChampion/plugin)
set (DOC_INSTALL_DIR ${INSTALL_DIR}/${CMAKE_INSTALL_DATAROOTDIR}/doc)

set (DEFAULT_CLIENT_LIB_TGT "cc_mqttsn_client")

######################################################################

add_subdirectory(client)
add_subdirectory(gateway)

