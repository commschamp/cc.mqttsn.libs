if (CMAKE_CROSSCOMPILING)
    return()
endif ()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_subdirectory (src)

install (
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/mqttsn
    DESTINATION ${INC_INSTALL_DIR}
)
