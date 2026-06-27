QT += core gui widgets opengl openglwidgets
CONFIG += c++17 console
CONFIG -= app_bundle

TARGET = Polo
TEMPLATE = app

SOURCES += \
    src/app/main.cpp \
    src/core/CommandParser.cpp \
    src/core/History.cpp \
    src/core/PathTools.cpp \
    src/core/PtySession.cpp \
    src/core/ShellSession.cpp \
    src/render/Clip3DWidget.cpp \
    src/ui/TerminalHighlighter.cpp \
    src/ui/TerminalTab.cpp \
    src/ui/TerminalView.cpp \
    src/ui/TerminalWindow.cpp \
    src/ui/TopBar.cpp

HEADERS += \
    src/core/CommandParser.h \
    src/core/History.h \
    src/core/PathTools.h \
    src/core/PtySession.h \
    src/core/ShellSession.h \
    src/render/Clip3DWidget.h \
    src/ui/TerminalHighlighter.h \
    src/ui/TerminalTab.h \
    src/ui/TerminalView.h \
    src/ui/TerminalWindow.h \
    src/ui/TopBar.h

INCLUDEPATH += src

RESOURCES += assets/polo.qrc
