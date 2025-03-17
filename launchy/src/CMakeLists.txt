set(launchy_src 
    main.cpp
    globals.cpp
    options.cpp
    catalog.cpp
    catalog_builder.cpp
    plugin_handler.cpp
    platform_base_hotkey.cpp
    icon_delegate.cpp
    plugin_interface.cpp
    catalog_types.cpp
    icon_extractor.cpp
    Fader.cpp
    CharListWidget.cpp
    CharLineEdit.cpp
    CommandHistory.cpp
    InputDataList.cpp
    FileSearch.cpp
    AnimationLabel.cpp
    SettingsManager.cpp
    precompiled.cpp
)

set(launchy_ui
    options.ui
)

set(launchy_rcc
    ""
)

set(launchy_platform_src "")

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    enable_language("RC")
    set (launchy_rcc ${launchy_rcc} ${CMAKE_SOURCE_DIR}/launchy/win/launchy.rc)
    set (launchy_platform_src
        ${CMAKE_SOURCE_DIR}/launchy/platforms/win/platform_win.cpp
        ${CMAKE_SOURCE_DIR}/launchy/platforms/win/platform_win_hotkey.cpp
        ${CMAKE_SOURCE_DIR}/launchy/platforms/win/platform_win_util.cpp
        ${CMAKE_SOURCE_DIR}/launchy/platforms/win/WinIconProvider.cpp
        ${CMAKE_SOURCE_DIR}/launchy/platforms/win/minidump.cpp    
    )
endif()

# This needs to be manually wrapped since it doesn't respect the naming convention
qt_wrap_cpp(launchy_platform_src platform_base_hottrigger.h) 

qt_add_executable(launchy
    ${launchy_src}
    ${launchy_platform_src}
    ${launchy_ui}
    ${launchy_rcc}
)

target_link_libraries(launchy PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network common_lib)
target_include_directories(launchy PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(launchy PROPERTIES
    WIN32_EXECUTABLE $<$<CONFIG:DEBUG>:OFF>$<$<CONFIG:RELEASE>:ON>
    MACOSX_BUNDLE ON   
)

qt_add_translations(launchy
    TS_FILE_DIR ${CMAKE_SOURCE_DIR}/launchy/translations
    TS_FILE_BASE launchy
)

qt_add_resources(launchy launchy_rcc
    PREFIX "/"
    FILES 
        resources/launchy128.png 
        resources/launchy16.png 
        resources/basicskin.qss
)

qt_add_resources(launchy launchy_rcc_settings
    PREFIX "/settings"
    FILES 
        resources/about.png 
        resources/catalog.png
        resources/general.png
        resources/plugins1.png
        resources/plugins2.png
        resources/skins.png
        resources/advanced.png
)