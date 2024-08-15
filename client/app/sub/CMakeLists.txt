set (name "cc_mqttsn_client_sub")
set (src
    main.cpp
    Sub.cpp
)

add_executable(${name} ${src})
target_link_libraries(${name} ${COMMON_APPS_LIB})

install (
    TARGETS ${name}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)