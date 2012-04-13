
#include <QDebug>
#include <QQuickView>
#include <QQmlEngine>
#include <QQmlContext>
#include <QQuickItem>

#include <QGuiApplication>

#include "qeventbus.h"
#include "qcommonjsmodule.h"
#include "qbuffermodule.h"
#include "qstdlibmodule.h"
#include "qworkermodule.h"
#include "qschedulingmodule.h"

#include "imagemodule.h"
#include "qvfsmodule.h"
#include "conversionoutput.h"

int main(int argc, char *argv[])
{
    QGuiApplication a(argc, argv);

    QEventBus eventBus; // Used for communication between QML and JS

    QQuickView quickView;
    quickView.engine()->setOutputWarningsToStandardError(true);
    quickView.setResizeMode(QQuickView::SizeRootObjectToView);
    quickView.rootContext()->setContextProperty("eventbus", &eventBus);
    quickView.setSource(QUrl("converter/gui/gui.qml"));
    quickView.show();

    QQuickItem *rootItem = quickView.rootObject();

    QCommonJSModule commonJsModule(quickView.engine(), QDir("converter/"));

    QBufferModule::install(&commonJsModule);
    QStdLibModule::install(&commonJsModule);

    QWorkerModule *workerModule = new QWorkerModule(quickView.engine());
    commonJsModule.addNativeModule("eventbus", &eventBus);
    commonJsModule.addNativeModule("worker", workerModule);
    commonJsModule.addNativeModule("vfs", new QVfsModule(&commonJsModule));
    commonJsModule.addNativeModule("image", new ImageModule(&commonJsModule));
    commonJsModule.addNativeModule("conversion/output", new ConversionOutput(&commonJsModule));
    commonJsModule.addNativeModule("scheduling", new QSchedulingModule(quickView.engine()));

    /* Call startup() method on the startup module */
    QJSValue startupModule = commonJsModule.require("startup");
    startupModule.property("startup").call(QJSValueList() << quickView.engine()->newQObject(rootItem));

    return a.exec();
}
