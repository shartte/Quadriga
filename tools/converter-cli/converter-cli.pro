#-------------------------------------------------
#
# Project created by QtCreator 2012-03-04T23:58:13
#
#-------------------------------------------------


QT       += core qml v8-private quick

TARGET = converter-cli
CONFIG   += console link_prl
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../conversion/release/ -lconversion
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../conversion/debug/ -lconversion
else:symbian: LIBS += -lconversion
else:unix: LIBS += -L$$OUT_PWD/../conversion/ -lconversion

INCLUDEPATH += $$PWD/../conversion
DEPENDPATH += $$PWD/../conversion

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../conversion/release/conversion.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../conversion/debug/conversion.lib
else:unix:!symbian: PRE_TARGETDEPS += $$OUT_PWD/../conversion/libconversion.a

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../troikaformats/release/ -ltroikaformats
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../troikaformats/debug/ -ltroikaformats
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

OTHER_FILES += \
    converter/startup.js \
    converter/log.js \
    converter/conversion.js \
    converter/conversion/images.js \
    converter/gui/ProgressBar.qml \
    converter/gui/PathChooser.qml \
    converter/gui/gui.qml \
    converter/gui/Conversion.qml \
    converter/gui/Button.qml \
    converter/conversion/util.js \
    converter/control.js \
    converter/conversion/materials.js \
    converter/plan.js \
    converter/plan.js \
    converter/conversion/material.js
