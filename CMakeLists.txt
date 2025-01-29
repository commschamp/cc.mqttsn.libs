cmake_minimum_required (VERSION 3.10)
project ("cc_mqttsn_libs")

option (CC_MQTTSN_WARN_AS_ERR "Do NOT treat warning as error" ON)
option (CC_MQTTSN_CLIENT_DEFAULT_LIB "Build and install default variant of MQTT-SN Client library" ON)
option (CC_MQTTSN_CLIENT_LIB_FORCE_PIC "Force Position Independent Code (PIC) when compiling client library(ies)" OFF)
option (CC_MQTTSN_CLIENT_APPS "Build and install client applications" ${CC_MQTTSN_CLIENT_DEFAULT_LIB})
option (CC_MQTTSN_GATEWAY_LIB "Build and install MQTT-SN gateway library" ON)
option (CC_MQTTSN_GATEWAY_LIB_FORCE_PIC "Force Position Independent Code (PIC) when compiling gateway library" OFF)
option (CC_MQTTSN_GATEWAY_APPS "Build and install gateway applications" ${CC_MQTTSN_GATEWAY_LIB})
option (CC_MQTTSN_AFL_FUZZ "Build and install AFL++ fuzzing application(s)" OFF)
option (CC_MQTTSN_BUILD_UNIT_TESTS "Build unittests." OFF)
option (CC_MQTTSN_UNIT_TEST_WITH_VALGRIND "Run unittests with valgrind." OFF)
option (CC_MQTTSN_USE_CCACHE "Use ccache on unix system" OFF)
option (CC_MQTTSN_WITH_SANITIZERS "Build with sanitizers" OFF)

# CMake built-in options
option (BUILD_SHARED_LIBS "Build as shared libraries" OFF)

# Extra variables
# CC_MQTTSN_CUSTOM_CLIENT_CONFIG_FILES - List of custom client configuration files

##########################################################################

if ("${CMAKE_CXX_STANDARD}" STREQUAL "")
    set(CMAKE_CXX_STANDARD 17)
endif ()

if (("${CMAKE_VERSION}" VERSION_GREATER_EQUAL "3.30") AND
    (NOT DEFINED CMAKE_POLICY_DEFAULT_CMP0167))
    # Find boost cmake configuration from the boost installation
    cmake_policy(SET CMP0167 NEW)
endif ()

find_package(LibComms REQUIRED)
find_package(cc_mqttsn REQUIRED)

include (${PROJECT_SOURCE_DIR}/cmake/Compile.cmake)
cc_mqttsn_compile ()

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

