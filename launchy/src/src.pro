TEMPLATE = app
win32:TARGET = Launchy

CONFIG += debug_and_release
PRECOMPILED_HEADER = precompiled.h

QT += network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets winextras

# Check for requried environment variables
!exists($$(BOOST_DIR)) {
 error("The BOOST_DIR environment variable is not defined.")
}

INCLUDEPATH += ../common
INCLUDEPATH += $$(BOOST_DIR)

TR_EXCLUDE += $$(BOOST_DIR)/*

SOURCES = main.cpp \
    globals.cpp \
    options.cpp \
    catalog.cpp \
    catalog_builder.cpp \
    plugin_handler.cpp \
    platform_base_hotkey.cpp \
    icon_delegate.cpp \
    plugin_interface.cpp \
    catalog_types.cpp \
    icon_extractor.cpp \
    ../common/FileBrowserDelegate.cpp \
    ../common/FileBrowser.cpp \
    ../common/DropListWidget.cpp \
    Fader.cpp \
    CharListWidget.cpp \
    CharLineEdit.cpp \
    CommandHistory.cpp \
    InputDataList.cpp \
    FileSearch.cpp \
    AnimationLabel.cpp \
        SettingsManager.cpp \
    precompiled.cpp
HEADERS = platform_base.h \
    globals.h \
    main.h \
    catalog.h \
    catalog_builder.h \
    plugin_interface.h \
    plugin_handler.h \
    options.h \
    catalog_types.h \
    icon_delegate.h \
    icon_extractor.h \
    ../common/FileBrowserDelegate.h \
    ../common/FileBrowser.h \
    ../common/DropListWidget.h \
    CharListWidget.h \
    CharLineEdit.h \
    Fader.h \
    precompiled.h \
    CommandHistory.h \
    InputDataList.h \
    FileSearch.h \
    AnimationLabel.h \
        SettingsManager.h
FORMS = options.ui

win32 {
    ICON = Launchy.ico
    if(!debug_and_release|build_pass):CONFIG(debug, debug|release):CONFIG += console
    SOURCES += ../platforms/win/platform_win.cpp \
        ../platforms/win/platform_win_hotkey.cpp \
        ../platforms/win/platform_win_util.cpp \
        ../platforms/win/WinIconProvider.cpp \
        ../platforms/win/minidump.cpp
    HEADERS += ../platforms/win/WinIconProvider.h \
        platform_base_hotkey.h \
        platform_base_hottrigger.h \
        ../platforms/win/platform_win.h \
        ../platforms/win/platform_win_util.h \
        ../platforms/win/minidump.h
    CONFIG += embed_manifest_exe

    RC_FILE = ../win/launchy.rc
        LIBS += shell32.lib \
                user32.lib \
                gdi32.lib \
                ole32.lib \
                comctl32.lib \
                advapi32.lib \
                userenv.lib \
        netapi32.lib
    DEFINES = VC_EXTRALEAN \
        WIN32 \
        _UNICODE \
        UNICODE \
        WINVER=0x0600 \
        _WIN32_WINNT=0x0600 \
        _WIN32_WINDOWS=0x0600 \
        _WIN32_IE=0x0700
    if(!debug_and_release|build_pass) {
        CONFIG(debug, debug|release):DESTDIR = ../debug/
        CONFIG(release, debug|release):DESTDIR = ../release/
    }

    DEFINES += ENABLE_DEBUG_LOG
    QMAKE_CXXFLAGS_RELEASE += /Zi
    QMAKE_LFLAGS_RELEASE += /DEBUG
}

#TRANSLATIONS = ../translations/launchy_fr.ts \
#    ../translations/launchy_nl.ts \
#    ../translations/launchy_zh.ts \
#    ../translations/launchy_es.ts \
#    ../translations/launchy_de.ts \
#    ../translations/launchy_ja.ts \
#    ../translations/launchy_zh_TW.ts \
#    ../translations/launchy_rus.ts \
#    ../translations/launchy_it_IT.ts \
OBJECTS_DIR = build
MOC_DIR = build
RESOURCES += launchy.qrc


# if(!debug_and_release|build_pass):CONFIG(debug, debug|release): DEFINES += ENABLE_LOG_FILE