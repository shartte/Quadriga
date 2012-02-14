#-------------------------------------------------
#
# Project created by QtCreator 2012-02-12T16:36:20
#
#-------------------------------------------------

QT       += core declarative quick quick-private concurrent

TARGET = qt5sandbox
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    gamesystem.cpp

OTHER_FILES += \
    Test.qml \
    Loading.qml \
    ajax-loader.gif

HEADERS += \
    gamesystem.h

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../OgreSDK_vc10_v1-7-4/lib/release/ -lOgreMain
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../OgreSDK_vc10_v1-7-4/lib/debug/ -lOgreMain_d

win32:LIBS += -L$$PWD/../../OgreSDK_vc10_v1-7-4/boost_1_47/lib/

INCLUDEPATH += $$PWD/../../OgreSDK_vc10_v1-7-4/include/Ogre $$PWD/../../OgreSDK_vc10_v1-7-4/boost_1_47
DEPENDPATH += $$PWD/../../OgreSDK_vc10_v1-7-4/include/Ogre
