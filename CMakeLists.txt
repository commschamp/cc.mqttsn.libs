cmake_minimum_required (VERSION 3.1)
project ("mqttsn")

option (CC_MQTTSN_NO_WARN_AS_ERR "Do NOT treat warning as error" OFF)
option (CC_MQTTSN_CLIENT_DEFAULT_LIB "Build and install default variant of MQTT-SN Client library" ON)
option (CC_MQTTSN_BUILD_GATEWAY "Build and install MQTT-SN client library(ies) and applications" ON)
option (CC_MQTTSN_NO_UNIT_TESTS "Disable unittests." OFF)
option (CC_MQTTSN_EXTERNALS_UPDATE_DISCONNECTED "Allow skip of external projects update." OFF)

# Extra variables
# CC_MAIN_INSTALL_DIR - Path to external installation directory of comms_champion project
# CC_MQTT311_GENERATED_INSTALL_DIR - Path to external installation directory of cc.mqtt311.generated project
# CC_MQTTSN_CUSTOM_CLIENT_CONFIG_FILES - List of custom client configuration files
# CC_MQTTSN_DEFAULT_CLIENT_CONFIG_FILE - Custom congiruation of the default client.

if (CMAKE_TOOLCHAIN_FILE AND EXISTS ${CMAKE_TOOLCHAIN_FILE})
    message(STATUS "Loading toolchain from ${CMAKE_TOOLCHAIN_FILE}")
endif()

if (NOT CMAKE_CXX_STANDARD)
    set (CMAKE_CXX_STANDARD 11)
endif()

set (EXTERNALS_DIR "${PROJECT_SOURCE_DIR}/externals")
set (PROTOCOL_INC_DIR "${PROJECT_SOURCE_DIR}/protocol/include")
set (CXXTEST_EXTERNAL_DIR "${EXTERNALS_DIR}/cxxtest")

while (TRUE)
    if (CC_MQTTSN_NO_UNIT_TESTS)
        message (STATUS "Unittests are disabled")
        break()
    endif ()
    
    if (CMAKE_CROSSCOMPILING)
        message (STATUS "Not building unittests when cross-compiling")
        break()
    endif ()
    
    find_package (Git)
    if (NOT GIT_FOUND)
        message (WARNING "Git is required to check out cxxtest")
        break()
    endif ()
    
    set (cxx_tag "4.4")
    set (cxxtest_stamp "${EXTERNALS_DIR}/cxxtest.stamp")
    if ((EXISTS ${cxxtest_stamp}) AND (EXISTS "${CXXTEST_EXTERNAL_DIR}"))
        message (STATUS "Updating cxxtest")
        execute_process (
            COMMAND ${GIT_EXECUTABLE} checkout ${cxx_tag}
            WORKING_DIRECTORY ${CXXTEST_EXTERNAL_DIR}
        )
    else ()
        execute_process (
            COMMAND ${CMAKE_COMMAND} -E remove_directory "${CXXTEST_EXTERNAL_DIR}"
        )
        
        execute_process (
            COMMAND ${CMAKE_COMMAND} -E make_directory "${EXTERNALS_DIR}"
        )

        message (STATUS "Cloning cxxtest")
        execute_process (
            COMMAND ${GIT_EXECUTABLE} clone -b ${cxx_tag} --depth 1 https://github.com/CxxTest/cxxtest.git
            WORKING_DIRECTORY ${EXTERNALS_DIR}
            RESULT_VARIABLE git_result
        )

        if (NOT "${git_result}" STREQUAL "0")
            message (WARNING "git clone/checkout failed")
            break()
        endif ()

        execute_process (
            COMMAND ${CMAKE_COMMAND} -E touch "${cxxtest_stamp}"
        )
    endif ()

    set (CXXTEST_INCLUDE_DIR "${CXXTEST_EXTERNAL_DIR}")
    set (CXXTEST_PYTHON_TESTGEN_EXECUTABLE "${CXXTEST_INCLUDE_DIR}/bin/cxxtestgen")
    
    find_package (CxxTest)
    if (NOT CXXTEST_FOUND)
        message (WARNING "Wrong cxxtest paths, must be fixed")
        break ()
    endif ()
    
    enable_testing ()
    break()
endwhile ()

if (UNIX AND (NOT CC_MQTTSN_NO_CCACHE))
    find_program(CCACHE_FOUND ccache)
    if(CCACHE_FOUND)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
    endif(CCACHE_FOUND)
endif ()    

if ((CMAKE_COMPILER_IS_GNUCC) OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"))
    set (extra_flags_list
        "-Wall" "-Wextra" "-Wcast-align" "-Wcast-qual" "-Wctor-dtor-privacy"
        "-Wmissing-include-dirs"
        "-Woverloaded-virtual" "-Wredundant-decls" "-Wshadow" "-Wundef" "-Wunused"
        "-Wno-unknown-pragmas" "-fdiagnostics-show-option"
    )

    if (CMAKE_COMPILER_IS_GNUCC)
        list (APPEND extra_flags_list
            "-Wnoexcept" "-Wlogical-op" "-Wstrict-null-sentinel"
        )
    endif ()

    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        list (APPEND extra_flags_list "-Wno-dangling-field" "-Wno-unused-command-line-argument")
    endif ()

    if (NOT CC_MQTTSN_NO_WARN_AS_ERR)
        list (APPEND extra_flags_list "-Werror")
    endif ()
    
    string(REPLACE ";" " " extra_flags "${extra_flags_list}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${extra_flags}")
    
    set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined") 
elseif (MSVC)
    add_definitions( "/wd4503" "/wd4309" "/wd4267" "-D_SCL_SECURE_NO_WARNINGS")
    
    if (NOT CC_NO_WARN_AS_ERR)
        add_definitions("/WX")
    endif ()
endif ()

set (INSTALL_DIR ${CMAKE_INSTALL_PREFIX})
#if (NOT "${CC_MQTTSN_INSTALL_DIR}" STREQUAL "")
#    set (INSTALL_DIR "${CC_MQTTSN_INSTALL_DIR}")
#endif ()

include(GNUInstallDirs)
set (INSTALL_NAME "CC_Mqttsn")
set (LIB_INSTALL_DIR ${INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR})
set (BIN_INSTALL_DIR ${INSTALL_DIR}/${CMAKE_INSTALL_BINDIR})
set (ETC_INSTALL_DIR ${INSTALL_DIR}/${CMAKE_INSTALL_SYSCONFDIR})
set (INC_INSTALL_DIR ${INSTALL_DIR}/${CMAKE_INSTALL_INCLUDEDIR})
set (CONFIG_INSTALL_DIR ${INSTALL_DIR}/config)
set (PLUGIN_INSTALL_DIR ${INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}/CommsChampion/plugin)
set (DOC_INSTALL_DIR ${INSTALL_DIR}/doc)

set (DEFAULT_CLIENT_LIB_TGT "cc_mqttsn_client")

if (NOT "${CC_MQTTSN_QT_DIR}" STREQUAL "")
    list (APPEND CMAKE_PREFIX_PATH ${CC_MQTTSN_QT_DIR})
endif ()

######################################################################

set (CC_EXTERNAL_DIR "${EXTERNALS_DIR}/comms_champion")
set (CC_EXTERNAL_TGT "comms_champion_external")
if ("${CC_MAIN_INSTALL_DIR}" STREQUAL "")
    execute_process (
        COMMAND ${CMAKE_COMMAND} -E make_directory "${EXTERNALS_DIR}")
        
        
    include(ExternalProject)
    
    set (cc_tag "develop")
    set (cc_repo "https://github.com/arobenko/comms_champion.git")
    set (cc_main_dir "${CMAKE_CURRENT_BINARY_DIR}/comms_champion")
    set (cc_src_dir "${CC_EXTERNAL_DIR}")
    set (cc_bin_dir "${cc_main_dir}/build")
    set (cc_install_dir "${cc_main_dir}/install")
    set (cc_update_disconnected)
    if (CC_MQTTSN_EXTERNALS_UPDATE_DISCONNECTED)
        set (cc_update_disconnected UPDATE_DISCONNECTED 1)
    endif ()
    
    find_package (Git REQUIRED)
    set (ext_targets_inc_file "${cc_src_dir}/cmake/CC_DefineExternalProjectTargets.cmake")
    if (NOT EXISTS "${ext_targets_inc_file}")
        execute_process (
            COMMAND ${CMAKE_COMMAND} -E remove_directory "${cc_src_dir}"
        )

        execute_process (
            COMMAND 
                ${GIT_EXECUTABLE} clone -b ${cc_tag} --depth 1 ${cc_repo} ${cc_src_dir}
            RESULT_VARIABLE git_result
        )

        if (NOT "${git_result}" STREQUAL "0")
            message (FATAL_ERROR "git clone/checkout failed")
        endif ()

        if (NOT EXISTS "${ext_targets_inc_file}")
             message (FATAL_ERROR "Incompatible version of ${cc_repo}, please use different tag.")
        endif ()

    endif ()
    
    ExternalProject_Add(
        "${CC_EXTERNAL_TGT}"
        PREFIX "${cc_bin_dir}"
        STAMP_DIR "${cc_bin_dir}"    
        GIT_REPOSITORY "${cc_repo}"
        GIT_TAG "${cc_tag}"
        SOURCE_DIR "${cc_src_dir}"
        CMAKE_ARGS 
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_INSTALL_PREFIX=${cc_install_dir} 
            -DCC_NO_UNIT_TESTS=ON -DCC_NO_WARN_AS_ERR=ON -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
            -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} 
            -DCC_COMMS_LIB_ONLY=ON
        BINARY_DIR "${cc_bin_dir}"
        ${cc_update_disconnected}
    )
    
    include (${ext_targets_inc_file})
    cc_define_external_project_targets(${cc_install_dir} NO_COMMS_CHAMPION)
else ()
    list (APPEND CMAKE_PREFIX_PATH "${CC_MAIN_INSTALL_DIR}")
    find_package(LibComms NO_MODULE)
    if (NOT TARGET cc::comms)
        message (FATAL_ERROR "Failed to find external COMMS library")
    endif ()
endif ()

######################################################################

include_directories(protocol/include)

add_subdirectory(client)
add_subdirectory(gateway)

