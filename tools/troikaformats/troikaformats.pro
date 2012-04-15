
TARGET = troikaformats
TEMPLATE = lib
CONFIG += staticlib

DEFINES += TROIKAFORMATS_LIBRARY

HEADERS += \
    include/troikaformats/archive.h \
    include/troikaformats/color.h \
    include/troikaformats/archiveentry.h \
    include/troikaformats/gamepathguesser.h \
    src/stable.h \
    include/troikaformats/targaimage.h \
    include/troikaformats/skeleton.h \
    include/troikaformats/model.h \
    include/troikaformats/util.h \
    include/troikaformats/constants.h \
    include/troikaformats/skmreader.h \
    include/troikaformats/dagreader.h

SOURCES += \
    src/archive.cpp \
    src/targaimage.cpp \
    src/skeleton.cpp \
    src/model.cpp \
    src/skmreader.cpp \
    src/dagreader.cpp

win32:SOURCES += src/gamepathguesser_win32.cpp
else:SOURCES += src/gamepathguesser.cpp

INCLUDEPATH += include/
PRECOMPILED_HEADER = src/stable.h
