
exists($$(OGRE_HOME)) {
    message(Using OGRE from $$(OGRE_HOME))

    INCLUDEPATH += $$(OGRE_HOME)/include/OGRE/ $$(OGRE_HOME)/boost_1_47/
    LIBS += -L$$(OGRE_HOME)/boost_1_47/lib/
    CONFIG(debug, debug|release) {
        LIBS += -L$$(OGRE_HOME)/lib/debug/ -lOgreMain_d
    } else {
        LIBS += -L$$(OGRE_HOME)/lib/release/ -lOgreMain
    }

} else {
    error("You must set the environment variable OGRE_HOME to the directory where you installed your Ogre SDK.")
}
