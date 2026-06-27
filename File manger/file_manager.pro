QT       += core gui widgets

CONFIG += c++17

INCLUDEPATH += include

SOURCES += \
    src/main.cpp \
    src/MainWindow.cpp \
    src/TextEditor.cpp \
    src/ImageViewer.cpp

HEADERS += \
    include/MainWindow.h \
    include/TextEditor.h \
    include/ImageViewer.h

TARGET = file_manager
TEMPLATE = app
