#-------------------------------------------------
#
# Project created by QtCreator 2012-03-04T12:04:57
#
#-------------------------------------------------

QT       += testlib

QT       -= gui

TARGET = tst_materialfiletest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += tst_materialfiletest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../troikaformats/release/ -ltroikaformats
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../troikaformats/debug/ -ltroikaformats
else:unix:!symbian: LIBS += -L$$OUT_PWD/../../troikaformats/ -ltroikaformats

INCLUDEPATH += $$PWD/../../troikaformats/include
DEPENDPATH += $$PWD/../../troikaformats/include

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../troikaformats/release/troikaformats.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../troikaformats/debug/troikaformats.lib
else:unix:!symbian: PRE_TARGETDEPS += $$OUT_PWD/../../troikaformats/libtroikaformats.a
