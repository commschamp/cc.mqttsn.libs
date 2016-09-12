if (CMAKE_CROSSCOMPILING)
    return ()
endif ()

if (NOT CXXTEST_FOUND)
    return ()
endif ()

set (COMPONENT_NAME "mqttsn.gateway")

#################################################################

function (test_func test_suite_name)
    set (tests "${CMAKE_CURRENT_SOURCE_DIR}/${test_suite_name}.th")

    set (name "${COMPONENT_NAME}.${test_suite_name}Test")

    set (runner "${test_suite_name}TestRunner.cpp")
    
    CXXTEST_ADD_TEST (${name} ${runner} ${tests} ${extra_sources})
    target_link_libraries(${name} ${MQTTSN_GATEWAY_LIB_NAME})
    
endfunction ()

#################################################################

function (test_gateway)
    test_func ("Gateway")
endfunction ()

#################################################################

include_directories (
    "${CXXTEST_INCLUDE_DIR}"
)

test_gateway()