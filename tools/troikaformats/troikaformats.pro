
QT       -= gui

TARGET = troikaformats
TEMPLATE = lib
CONFIG += staticlib

DEFINES += TROIKAFORMATS_LIBRARY

HEADERS += \
    include/troikaformats/materialfile.h \
    include/troikaformats/materialdefinition.h \
    include/troikaformats/archive.h \
    include/troikaformats/color.h \
    include/troikaformats/archiveentry.h \
    include/troikaformats/gamepathguesser.h \
    src/stable.h \
    src/materialcommand.h \
    include/troikaformats/targaimage.h

SOURCES += \
    src/materialfile.cpp \
    src/archive.cpp \
    src/targaimage.cpp

win32:SOURCES += src/gamepathguesser_win32.cpp
else:SOURCES += src/gamepathguesser.cpp

INCLUDEPATH += include/
PRECOMPILED_HEADER = src/stable.h
