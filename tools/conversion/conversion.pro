#-------------------------------------------------
#
# Project created by QtCreator 2012-03-04T23:45:51
#
#-------------------------------------------------

QT      += v8-private qml quick
QT       -= gui

TARGET = conversion
TEMPLATE = lib
CONFIG += staticlib create_prl

SOURCES += conversion.cpp \
    qvfsmodule.cpp \
    imagemodule.cpp \
    conversionoutput.cpp

HEADERS += conversion.h \
    stable.h \
    qvfsmodule.h \
    imagemodule.h \
    conversionoutput.h

PRECOMPILED_HEADER = stable.h

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../troikaformats/release/ -ltroikaformats
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../troikaformats/debug/ -ltroikaformats
else:symbian: LIBS += -ltroikaformats
else:unix: LIBS += -L$$OUT_PWD/../troikaformats/ -ltroikaformats

INCLUDEPATH += $$PWD/../troikaformats/include
DEPENDPATH += $$PWD/../troikaformats/include

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../troikaformats/release/troikaformats.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../troikaformats/debug/troikaformats.lib
else:unix:!symbian: PRE_TARGETDEPS += $$OUT_PWD/../troikaformats/libtroikaformats.a

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../scripting/release/ -lscripting
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../scripting/debug/ -lscripting
else:unix:!symbian: LIBS += -L$$OUT_PWD/../../scripting/ -lscripting

INCLUDEPATH += $$PWD/../../scripting
DEPENDPATH += $$PWD/../../scripting

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../scripting/release/scripting.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../scripting/debug/scripting.lib
else:unix:!symbian: PRE_TARGETDEPS += $$OUT_PWD/../../scripting/libscripting.a
