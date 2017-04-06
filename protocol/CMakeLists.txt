install (
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/mqttsn
    DESTINATION ${INC_INSTALL_DIR}
)

FILE(GLOB_RECURSE headers "*.h")
add_custom_target(mqttsn_protocol.headers SOURCES ${headers})
