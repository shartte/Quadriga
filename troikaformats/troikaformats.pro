
QT       -= gui

TARGET = TroikaFormats
TEMPLATE = lib
CONFIG += staticlib

DEFINES += TROIKAFORMATS_LIBRARY

HEADERS += \
    include/troikaformats/materialfile.h \
    include/troikaformats/materialdefinition.h \
    src/stable.h \
    include/troikaformats/archive.h \
    include/troikaformats/color.h \
    src/materialcommand.h

INCLUDEPATH += include/

SOURCES += \
    src/materialfile.cpp \
    src/archive.cpp

PRECOMPILED_HEADER = src/stable.h
