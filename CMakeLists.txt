cmake_minimum_required(VERSION 3.16)

project(launchy VERSION 3.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_PLUGINS "Build Plugins" ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)
find_package(Qt6 REQUIRED COMPONENTS LinguistTools)

qt_standard_project_setup(I18N_TRANSLATED_LANGUAGES fr nl zh es de ja zh_TW rus it_IT)

add_subdirectory(launchy)

## INSTALL
install(TARGETS launchy
    BUNDLE  DESTINATION ${CMAKE_INSTALL_BINDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_app_script(
    TARGET launchy
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
install(DIRECTORY launchy/skins DESTINATION ${CMAKE_INSTALL_BINDIR})
