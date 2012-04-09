
#include <QDebug>
#include <QFile>
#include <QtQml/private/qv8engine_p.h>

#include "qcommonjsmodule.h"

QCommonJSModule::QCommonJSModule(QJSEngine *engine, const QDir &scriptDir)
    : QObject(engine), mEngine(engine), mScriptDir(scriptDir), mCurrentModuleDir("")
{

    const char *wrapperScript = "(function(require, exports, module, moduleText) {\n"
            "var wrapper = new Function('require', 'exports', 'module', moduleText);\n"
            "wrapper.call({}, require, exports, module);\n"
            "})";

    mModuleWrapper = engine->evaluate(wrapperScript);

    if (engine->hasUncaughtException()) {
        QJSValue e = engine->uncaughtException();
        qDebug() << "Unable to create the CommonJS wrapper function." << e.toString();
    }

    QJSValue moduleObject = engine->newQObject(this);
    mRequireFunction = moduleObject.property("require");
}

void QCommonJSModule::addNativeModule(const QString &moduleId, QObject *module)
{
    Q_ASSERT(!mModuleCache.contains(moduleId));

    QJSValue value = mEngine->newQObject(module);
    mModuleCache[moduleId] = value;
}

void QCommonJSModule::addModule(const QString &moduleId, const QJSValue &value)
{
    Q_ASSERT(!mModuleCache.contains(moduleId));
    mModuleCache[moduleId] = value;
}

void QCommonJSModule::addModuleFromScript(const QString &moduleId, const QString &filename, const QString &script)
{
    QJSValue exports = mEngine->newObject();
    QJSValue module = mEngine->newObject();
    module.setProperty("id", moduleId);
    module.setProperty("uri", filename);
    module.setProperty("exports", exports);

    mModuleWrapper.call(QJSValueList() << mRequireFunction << exports << module << QJSValue(script));

    if (mEngine->hasUncaughtException()) {
        QJSValue uncaughtException = mEngine->uncaughtException();
        qDebug() << "Failed to load module " << uncaughtException.toString();
    } else {
        mModuleCache[moduleId] = module.property("exports");
    }
}

QJSValue QCommonJSModule::require(QString id)
{
    // Normalize relative module IDs
    id = normalizeModuleId(id);

    if (!mModuleCache.contains(id)) {
        QString filename = mScriptDir.absoluteFilePath(id + ".js");

        qDebug() << "Loading module" << id << "from" << filename;

        QV8Engine *v8Engine = mEngine->handle();

        QFile file(filename);
        if (!file.open(QFile::ReadOnly)) {
            qDebug() << "Module not found:" << filename;
            return v8Engine->scriptValueFromInternal(v8::ThrowException(v8::String::New("File not found...")));
        }

        QString script = QString::fromUtf8(file.readAll());
        file.close();

        QString oldCurrentDir = mCurrentModuleDir;
        if (id.contains("/")) {
            mCurrentModuleDir = id.left(id.lastIndexOf('/') + 1);
        } else {
            mCurrentModuleDir = "";
        }

        QJSValue exports = mEngine->newObject();
        QJSValue module = mEngine->newObject();
        module.setProperty("id", id);
        module.setProperty("uri", filename);
        module.setProperty("exports", exports);

        mModuleWrapper.call(QJSValueList() << mRequireFunction << exports << module << QJSValue(script));

        if (mEngine->hasUncaughtException()) {
            QJSValue uncaughtException = mEngine->uncaughtException();
            qDebug() << "Failed to load module " << uncaughtException.toString();
            return v8Engine->scriptValueFromInternal(v8::ThrowException(v8::String::New("Failed to load module")));
        }

        mModuleCache[id] = module.property("exports");

        mCurrentModuleDir = oldCurrentDir; // TODO: Safe-guard this with a guard or something...
    }

    return mModuleCache[id];
}

QString QCommonJSModule::normalizeModuleId(const QString &id) const
{
    if (!id.startsWith("./") && !id.startsWith("../"))
        return id;

    QString normalizedId = QDir::cleanPath(mCurrentModuleDir + id);
    return normalizedId;
}
