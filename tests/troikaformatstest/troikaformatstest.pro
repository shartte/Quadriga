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

INCLUDEPATH += ../../troikaformats/include/
HEADERS += ../../troikaformats/include/troikaformats/*
SOURCES += ../../troikaformats/src/*.cpp


SOURCES += tst_materialfiletest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
