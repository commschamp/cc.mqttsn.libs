if (NOT CMAKE_CROSSCOMPILING)
    return ()
endif ()

set (STARTUP_LIB_NAME "startup")
set (LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/raspberrypi.ld")

function (link_app tgt)

    set_target_properties (${tgt}
        PROPERTIES LINK_FLAGS "-Wl,-Map,kernel.map -T ${LINKER_SCRIPT}")
    
    add_custom_command (
        TARGET ${tgt}
        POST_BUILD
        COMMAND 
            ${CMAKE_OBJCOPY} $<TARGET_FILE:${tgt}> -O binary
                                        $<TARGET_FILE_DIR:${tgt}>/${tgt}.img
#        COMMAND 
#            ${CMAKE_OBJDUMP} -D -S $<TARGET_FILE:${tgt}> >
#                                        $<TARGET_FILE_DIR:${tgt}>/kernel.list
#        COMMAND 
#            ${CMAKE_OBJDUMP} -t $<TARGET_FILE:${tgt}> >
#                                        $<TARGET_FILE_DIR:${tgt}>/kernel.syms
#        COMMAND 
#            ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/image/${tgt}
#        COMMAND 
#            ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:${tgt}>/kernel.img ${CMAKE_BINARY_DIR}/image/${tgt}
    )
                                        
endfunction ()

add_subdirectory (startup)
add_subdirectory (test_client_build)

