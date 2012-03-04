
QT       -= gui

TARGET = TroikaFormats
TEMPLATE = static_lib

DEFINES += TROIKAFORMATS_LIBRARY

HEADERS += \
    include/troikaformats/materialfile.h \
    include/troikaformats/materialdefinition.h \
    src/stable.h \
    include/troikaformats/archive.h \
    include/troikaformats/color.h

INCLUDEPATH += include/

SOURCES += \
    src/materialfile.cpp \
    src/materialdefinition.cpp \
    src/archive.cpp

PRECOMPILED_HEADER = src/stable.h
