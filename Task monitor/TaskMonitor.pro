QT += core gui widgets
CONFIG += c++17

TARGET = TaskMonitor
TEMPLATE = app

SOURCES += \
    src/main.cpp \
    src/core/SystemInfo.cpp \
    src/ui/MainWindow.cpp \
    src/ui/CircularProgressBar.cpp

HEADERS += \
    src/core/SystemInfo.h \
    src/ui/MainWindow.h \
    src/ui/CircularProgressBar.h

RESOURCES += resources/resources.qrc
