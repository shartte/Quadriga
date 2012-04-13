#-------------------------------------------------
#
# Project created by QtCreator 2012-04-06T00:44:58
#
#-------------------------------------------------

QT       -= gui
QT       += qml qml-private v8-private core-private

TARGET = scripting
TEMPLATE = lib
CONFIG += staticlib create_prl

SOURCES += \
    qcommonjsmodule.cpp \
    qeventbus.cpp \
    qworkermodule.cpp \
    qbuffermodule.cpp \
    v8_buffer.cpp \
    qstdlibmodule.cpp \
    qschedulingmodule.cpp \
    qjsexceptionutils.cpp

HEADERS += \
    qcommonjsmodule.h \
    qeventbus.h \
    qworkermodule.h \
    qbuffermodule.h \
    v8_buffer.h \
    v8_object_wrap.h \
    qstdlibmodule.h \
    qvfsmodule.h \
    gamepathguesser.h \
    qschedulingmodule.h \
    qjsexceptionutils.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

RESOURCES += \
    resources.qrc

OTHER_FILES += \
    scripts/util.js \
    scripts/buffer.js \
    scripts/assert.js
