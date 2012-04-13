#ifndef QCOMMONJSMODULE_H
#define QCOMMONJSMODULE_H

#include <QJSEngine>
#include <QDir>
#include <QHash>

class QCommonJSModule : public QObject
{
Q_OBJECT
    Q_PROPERTY(QString dir READ scriptDir WRITE setScriptDir)
public:

    QCommonJSModule(QJSEngine *engine, const QDir &scriptDir);

    void setScriptDir(const QString &scriptDir) {
        mScriptDir = QDir(scriptDir);
    }

    QString scriptDir() const {
        return mScriptDir.absolutePath();
    }

    QJSEngine *engine() const {
        return mEngine;
    }

    void addNativeModule(const QString &moduleId, QObject *module);

    void addModule(const QString &moduleId, const QJSValue &value);

    void addModuleFromScript(const QString &moduleId, const QString &filename, const QString &script);

public slots:
    QJSValue require(QString id);

private:
    QString normalizeModuleId(const QString &id) const;

    QJSEngine *mEngine;
    QDir mScriptDir;
    QString mCurrentModuleDir;
    QHash<QString, QJSValue> mModuleCache;

    QJSValue mRequireFunction;
};

#endif // QCOMMONJSMODULE_H
