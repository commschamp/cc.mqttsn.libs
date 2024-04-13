cmake_minimum_required (VERSION 3.10)
project ("cc_mqttsn_libs")

option (CC_MQTTSN_WARN_AS_ERR "Do NOT treat warning as error" ON)
option (CC_MQTTSN_CLIENT_DEFAULT_LIB "Build and install default variant of MQTT-SN Client library" ON)
option (CC_MQTTSN_BUILD_GATEWAY "Build and install MQTT-SN client library(ies) and applications" ON)
option (CC_MQTTSN_BUILD_CLIENT_APPS "Build and install client applications" ${CC_MQTTSN_CLIENT_DEFAULT_LIB})
option (CC_MQTTSN_BUILD_GATEWAY_APPS "Build and install gateway applications" ${CC_MQTTSN_BUILD_GATEWAY})
option (CC_MQTTSN_BUILD_UNIT_TESTS "Build unittests." OFF)
option (CC_MQTTSN_UNIT_TEST_WITH_VALGRIND "Disable valgrind in unittests." OFF)
option (CC_MQTTSN_USE_CCACHE "Use ccache on unix system" ON)
option (CC_MQTTSN_WITH_SANITIZERS "Build with sanitizers" OFF)

# Extra variables
# CC_MQTTSN_CUSTOM_CLIENT_CONFIG_FILES - List of custom client configuration files
# CC_MQTTSN_DEFAULT_CLIENT_CONFIG_FILE - Custom congiruation of the default client.
# CC_MQTTSN_SANITIZERS - List of sanitizing options

##########################################################################

if ("${CMAKE_CXX_STANDARD}" STREQUAL "")
    set(CMAKE_CXX_STANDARD 11)
endif ()

find_package(LibComms REQUIRED)

set (compile_opts)
if (CC_MQTTSN_USE_CCACHE)
    list (APPEND compile_opts USE_CCACHE)
endif ()

if (CC_MQTTSN_WARN_AS_ERR)
    list (APPEND compile_opts WARN_AS_ERR)
endif ()

if (CC_MQTTSN_WITH_SANITIZERS)
    list (APPEND compile_opts DEFAULT_SANITIZERS)

    if (CMAKE_COMPILER_IS_GNUCC)
        # gcc gives false pasitive on vptr sanitizer
        list (APPEND compile_opts EXTRA -fno-sanitize=vptr)
    endif ()
endif ()

if (EXISTS ${LibComms_DIR}/CC_Compile.cmake)
    include (${LibComms_DIR}/CC_Compile.cmake)
    cc_compile(${compile_opts})
else ()
    message (WARNING "Unexpected COMMS cmake scripts installation path, cannot reuse compilation options")
endif ()

set (EXTERNALS_DIR "${PROJECT_SOURCE_DIR}/externals")

while (TRUE)
    if (NOT CC_MQTTSN_BUILD_UNIT_TESTS)
        break()
    endif ()

    message (STATUS "Unittests are enabled")
    
    if (CMAKE_CROSSCOMPILING)
        message (STATUS "Not building unittests when cross-compiling")
        break()
    endif ()

    if (NOT EXISTS ${LibComms_DIR}/CC_CxxtestFuncs.cmake)
        message (WARNING "Unexpected COMMS cmake scripts installation path, cannot unittest")
        break ()
    endif ()

    include (${LibComms_DIR}/CC_CxxtestFuncs.cmake)
    cc_get_cxxtest(INSTALL_PREFIX ${EXTERNALS_DIR})

    if ((NOT CC_MQTTSN_WITH_SANITIZERS) AND (CC_MQTTSN_UNIT_TEST_WITH_VALGRIND))
        find_program(VALGRIND_EXECUTABLE NAMES "valgrind")
    endif ()    
    
    break()
endwhile ()

include(GNUInstallDirs)
set (DEFAULT_CLIENT_LIB_TGT "cc_mqttsn_client")

######################################################################

add_subdirectory(client)
add_subdirectory(gateway)

