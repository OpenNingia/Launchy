qt_add_library(common_lib STATIC 
    FileBrowserDelegate.cpp
    FileBrowser.cpp
    DropListWidget.cpp
)

target_include_directories(common_lib PRIVATE ${CMAKE_SOURCE_DIR}/launchy/src)
target_include_directories(common_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(common_lib PUBLIC Qt::Widgets)