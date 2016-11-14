set (THIS_PLUGIN_NAME "mqttsn_client_filter")

######################################################################

function (cc_plugin_mqttsn_client_filter)
    set (name "${THIS_PLUGIN_NAME}")
    
    if (NOT Qt5Widgets_FOUND)
        message(WARNING "Can NOT build ${name} due to missing Qt5Widgets library")
        return()
    endif ()
    
    set (meta_file "${CMAKE_CURRENT_SOURCE_DIR}/mqttsn_client_filter.json")
    set (stamp_file "${CMAKE_CURRENT_BINARY_DIR}/refresh_stamp.txt")
    
    set (refresh_plugin_header TRUE)
    if ((NOT EXISTS ${stamp_file}) OR (${meta_file} IS_NEWER_THAN ${stamp_file}))
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_SOURCE_DIR}/Plugin.h)
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E touch ${stamp_file})
    endif ()
    
    
    set (src
        Plugin.cpp
        Filter.cpp
        widget/PubSubWidget.cpp
        widget/SubListWidget.cpp
        widget/FilterConfigWidget.cpp
    )
    
    set (hdr
        Plugin.h
        Filter.h
        widget/PubSubWidget.h
        widget/SubListWidget.h
        widget/FilterConfigWidget.h
    )
    
    qt5_wrap_cpp(
        moc
        ${hdr}
    )
    
    qt5_wrap_ui(
        ui
        ui/PubSubWidget.ui
        ui/SubListElementWidget.ui
        ui/SubListWidget.ui
        ui/FilterConfigWidget.ui
    )
    
    qt5_add_resources(resources ${CMAKE_CURRENT_SOURCE_DIR}/ui.qrc)
    
    add_library (${name} SHARED ${src} ${moc} ${ui} ${resources})
    target_link_libraries (${name} ${DEFAULT_CLIENT_LIB_TGT} ${CC_PLUGIN_LIBRARIES})
    qt5_use_modules(${name} Widgets Core)
    
    install (
        TARGETS ${name}
        DESTINATION ${PLUGIN_INSTALL_DIR})

endfunction()

######################################################################

find_package(Qt5Widgets)

include_directories(
    ${CMAKE_BINARY_DIR}/client/src/basic
    ${CMAKE_SOURCE_DIR}/client/include
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

cc_plugin_mqttsn_client_filter ()

if (CC_MQTTSN_FULL_SOLUTION)
    add_dependencies(${THIS_PLUGIN_NAME} ${CC_EXTERNAL_TGT})
endif ()
