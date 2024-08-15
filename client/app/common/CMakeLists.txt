set (src
    AppClient.cpp
    ProgramOptions.cpp
    Session.cpp
    UdpSession.cpp
)

add_library(${COMMON_APPS_LIB} STATIC ${src})
target_link_libraries(${COMMON_APPS_LIB} PUBLIC cc::cc_mqttsn_client Boost::system Boost::program_options ${EXTRA_BOOST_TARGETS} ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(
    ${COMMON_APPS_LIB} BEFORE
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)    

target_compile_options(${COMMON_APPS_LIB} PUBLIC
    $<$<CXX_COMPILER_ID:MSVC>:/D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS>
) 
