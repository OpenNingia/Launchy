include(FetchContent)
FetchContent_Declare(
    Boost
    GIT_REPOSITORY https://github.com/boostorg/boost.git
    GIT_TAG boost-1.87.0
)
FetchContent_MakeAvailable(Boost)

qt_add_plugin(calcy
              
              CLASS_NAME net.launchy.PluginInterface.CalcyPlugin
              calcy.cpp
              gui.cpp
              dlg.ui
)

target_include_directories(calcy PRIVATE ${CMAKE_SOURCE_DIR}/launchy/src)
target_link_libraries(calcy PRIVATE Qt6::Core Qt6::Widgets Boost::spirit)

install(TARGETS calcy
    LIBRARY  DESTINATION ${CMAKE_INSTALL_BINDIR}/plugins
)
