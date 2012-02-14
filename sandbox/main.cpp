

#include <QtGui/QGuiApplication>
#include <QtGui/QImage>
#include <QtDeclarative/QtDeclarative>
#include <QtQuick/QtQuick>
#include <QtQuick/private/qquickwindowmanager_p.h>
#include <QDebug>
#include <QtCore/QElapsedTimer>
#include <QtConcurrent/QtConcurrent>
#include <QtGui/QPlatformOpenGLContext>

#include <OgreRoot.h>
#include <OgreConfigFile.h>
#include <OgreResourceGroupManager.h>
#include <OgreCamera.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreEntity.h>
#include <OgreRenderWindow.h>

#include "gamesystem.h"

class UpdateableQuickView : public QQuickView {
    Q_OBJECT
public:
    bool event(QEvent *e);
};

bool UpdateableQuickView::event(QEvent *e)
{
    if (e->type() == QEvent::Paint) {
        if (rootItem()) {
            rootItem()->polish();
        }
        return true;
    }
    return QQuickView::event(e);
}

int main(int argc, char *argv[])
{
    QGuiApplication a(argc, argv);

    Ogre::Root* mRoot;
    Ogre::String mResourcesCfg;
    Ogre::String mPluginsCfg;

#ifdef _DEBUG
    mResourcesCfg = "resources_d.cfg";
    mPluginsCfg = "plugins_d.cfg";
#else
    mResourcesCfg = "resources.cfg";
    mPluginsCfg = "plugins.cfg";
#endif

    QImage logo;
    if (!logo.load("logo.png")) {
        qFatal("Unable to load logo.png.");
    }

    QQuickView *view = new UpdateableQuickView();
    view->show();

    view->setClearBeforeRendering(false);
    view->setResizeMode(QQuickView::SizeRootObjectToView);
    view->setSource(QUrl("Loading.qml"));

    // Create a new proxy-global object since QML overrides the global object
    // with a non-extensible one
    QJSEngine *engine = view->engine();
    QJSValue result = engine->evaluate("var myObj = {}; myObj.callMethod = function (s) { console.log(s); };");
 /*   qDebug() << "Result: " << result.toString();

    GameSystem *system = new GameSystem();
    QJSValue systemWrapper = engine->newQObject(system);

    qDebug() << systemWrapper.isQObject();

    QJSValue globalObject = engine->globalObject().prototype();
    globalObject.setProperty("myObj", systemWrapper);
    if (!globalObject.hasOwnProperty("myObj")) {
        qDebug() << "Setting failed.";
    }
    if (!engine->globalObject().hasOwnProperty("myObj")) {
        qDebug() << "Obj is copied...";
    }*/
    Ogre::SceneManager *mSceneMgr;
    bool ogreInitialized = false;

    a.processEvents(QEventLoop::AllEvents);

    view->openglContext()->makeCurrent(view);
    auto ctxId = wglGetCurrentContext();

    QFuture<bool> initResult = QtConcurrent::run([&]() -> bool {

        // construct Ogre::Root
        mRoot = new Ogre::Root(mPluginsCfg);

        Ogre::ConfigFile cf;
        cf.load(mResourcesCfg);

        // Go through all sections & settings in the file
        Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

        Ogre::String secName, typeName, archName;
        while (seci.hasMoreElements())
        {
            secName = seci.peekNextKey();
            Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
            Ogre::ConfigFile::SettingsMultiMap::iterator i;
            for (i = settings->begin(); i != settings->end(); ++i)
            {
                typeName = i->first;
                archName = i->second;
                Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                    archName, typeName, secName);
            }
        }


        if(!(mRoot->restoreConfig() || mRoot->showConfigDialog()))
        {
            return false;
        }

        qDebug() << "Using OpenGL Context ID: " << ctxId;
        qDebug() << "Using HWND: " << view->winId();

        mRoot->initialise(false);

        Ogre::NameValuePairList params;
        params["externalWindowHandle"] = Ogre::StringConverter::toString((unsigned int)view->winId());
        params["externalGLControl"] = Ogre::StringConverter::toString( (true) );
        params["externalGLContext"] = Ogre::StringConverter::toString( (ctxId) );
        auto mWindow = mRoot->createRenderWindow( "", view->width(), view->height(), false, &params );

        // initialise all resource groups
        Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

        // Create the SceneManager, in this case a generic one
        mSceneMgr = mRoot->createSceneManager("DefaultSceneManager");

        static Ogre::Camera* mCamera;
        // Create the camera
        mCamera = mSceneMgr->createCamera("PlayerCam");

        // Position it at 80 in Z direction
        mCamera->setPosition(Ogre::Vector3(0,0,80));
        // Look back along -Z
        mCamera->lookAt(Ogre::Vector3(0,0,-300));
        mCamera->setNearClipDistance(5);

        // Create one viewport, entire window
        static Ogre::Viewport* vp = mWindow->addViewport(mCamera);
        vp->setBackgroundColour(Ogre::ColourValue(0,0,0));

        // Alter the camera aspect ratio to match the viewport
        mCamera->setAspectRatio(
            Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));

        static Ogre::Entity* ogreHead = mSceneMgr->createEntity("Head", "ogrehead.mesh");
        Ogre::SceneNode* headNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        headNode->attachObject(ogreHead);

        // Set ambient light
        mSceneMgr->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));

        // Create a light
        Ogre::Light* l = mSceneMgr->createLight("MainLight");
        l->setPosition(20,80,50);

        glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
        glPushAttrib(GL_ALL_ATTRIB_BITS);

        QMetaObject::invokeMethod(view, "setSource", Q_ARG(QUrl, QUrl("Test.qml")));

        ogreInitialized = true;

        return true;

    });

    long frameCount = 0;
    QElapsedTimer lastUpdateTimer;
    lastUpdateTimer.start();

    QObject::connect(view, &QQuickView::beforeRendering, [=]() {

        static bool initializedOgre = false;

        QOpenGLContext *ctx = QOpenGLContext::currentContext();
        ctx->functions()->glUseProgram(0);

        /*
        glClearColor(1, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        QSGTexture *tex = view->createTextureFromImage(logo);
        tex->bind();
        glBegin(GL_QUADS);
        glTexCoord2i(0, 0);
        glVertex2i(-1, 1);
        glTexCoord2i(0, 1);
        glVertex2i(-1, -1);
        glTexCoord2i(1, 1);
        glVertex2i(1, -1);
        glTexCoord2i(1, 0);
        glVertex2i(1, 1);
        glEnd();

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();*/
        if (ogreInitialized) {
            ctx->functions()->glBindBuffer(GL_ARRAY_BUFFER, 0);
            ctx->functions()->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

            ctx->functions()->glBindFramebuffer(GL_FRAMEBUFFER, 0);
            ctx->functions()->glBindRenderbuffer(GL_RENDERBUFFER, 0);
            ctx->functions()->glUseProgram(0);

            glPopAttrib();
            glPopClientAttrib();

            mRoot->renderOneFrame();

            glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
            glPushAttrib(GL_ALL_ATTRIB_BITS);

            ctx->functions()->glBindBuffer(GL_ARRAY_BUFFER, 0);
            ctx->functions()->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

            ctx->functions()->glBindFramebuffer(GL_FRAMEBUFFER, 0);
            ctx->functions()->glBindRenderbuffer(GL_RENDERBUFFER, 0);
            ctx->functions()->glUseProgram(0);
        }
    });

    QObject::connect(view, &QQuickCanvas::afterRendering, [=]() mutable  {

        frameCount++;
        if (lastUpdateTimer.elapsed() >= 1000) {
        QMetaObject::invokeMethod(view, "setWindowTitle", Qt::QueuedConnection,
                                  Q_ARG(QString, QString("FPS: %1").arg(frameCount)));
            frameCount = 0;
            lastUpdateTimer.restart();
        }

        auto e = new QEvent(QEvent::Paint);
        QGuiApplication::postEvent(view, e);
    });

    return a.exec();
}

#include "main.moc"
