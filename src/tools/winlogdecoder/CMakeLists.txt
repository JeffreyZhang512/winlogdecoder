cmake_minimum_required(VERSION 3.5)

project(WinLogDecoder VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Svg Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Svg Widgets)

set(PROJECT_SOURCES
        main.cpp
        MainWindow.cpp
        MainWindow.h
        MainWindow.ui
        Settings.h
        Settings.cpp
        Settings.ui
        winlogdecoder.qrc
        winlogdecoder.rc
        src/DecoderCtrl/DecoderInterface.h
        src/DecoderCtrl/DecoderInterface.cpp
        src/DecoderCtrl/EtlDecoder.h
        src/DecoderCtrl/EtlDecoder.cpp
        src/DecoderCtrl/DecoderCtrl.h
        src/DecoderCtrl/DecoderCtrl.cpp
        src/DecoderCtrl/EvtxDecoder.h
        src/DecoderCtrl/EvtxDecoder.cpp
        src/DecoderGui/DecoderGui.h
        src/DecoderGui/DecoderGui.cpp
        src/DecoderGui/DecoderGui.ui
        ../../common/version.h
        ../../common/about/DialogAbout.h
        ../../common/about/DialogAbout.cpp
        ../../common/about/DialogAbout.ui
        ../../common/log/LogInterface.h
        ../../common/log/LogView.h
        ../../common/log/LogView.cpp
        ../../common/log/LogView.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(WinLogDecoder
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET WinLogDecoder APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(WinLogDecoder SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(WinLogDecoder
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_include_directories(WinLogDecoder PUBLIC
        .
        src/DecoderCtrl
        src/DecoderGui
        ../../common/
        ../../common/about
        ../../common/log
)


target_compile_definitions(WinLogDecoder PUBLIC
    APPLICATION_SHORT_NAME="WinLogDecoder"
    APPLICATION_NAME="Windows Log Decoder"
)

target_link_libraries(WinLogDecoder PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Svg
    Qt::Widgets
)

set_target_properties(WinLogDecoder PROPERTIES
    WIN32_EXECUTABLE TRUE
)

install(TARGETS WinLogDecoder
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(WinLogDecoder)
endif()
