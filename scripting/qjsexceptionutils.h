#ifndef QJSEXCEPTIONUTILS_H
#define QJSEXCEPTIONUTILS_H

#include <QJSValue>
#include <QJSEngine>

class QJSExceptionUtils
{
public:

    static QJSValue newError(QJSEngine *engine, const QString &message);

    static void handleUnhandledException(QJSEngine *engine);

private:
    QJSExceptionUtils();
};

#endif // QJSEXCEPTIONUTILS_H
