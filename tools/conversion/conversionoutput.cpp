
#include <QDebug>

#include "conversionoutput.h"

#include "qcommonjsmodule.h"
#include "qbuffermodule.h"

ConversionOutput::ConversionOutput(QCommonJSModule *commonJsModule) :
    QObject(commonJsModule), mCommonJsModule(commonJsModule)
{
}

QJSValue ConversionOutput::addBuffer(const QString &category, const QString &filename, const QJSValue &buffer)
{
    QByteArray data = QBufferModule::getData(buffer);

    QFileInfo fi(filename);
    fi.dir().mkpath(".");

    QFile file(filename);
    if (!file.open(QFile::WriteOnly))
    {
        qDebug() << "Unable to open" << filename;
        return QJSValue(); // TODO: Exception
    }

    file.write(data);

    return QJSValue();
}

QJSValue ConversionOutput::addString(const QString &category, const QString &filename, const QString &content)
{
    QFileInfo fi(filename);
    fi.dir().mkpath(".");

    QFile file(filename);
    if (!file.open(QFile::WriteOnly))
    {
        qDebug() << "Unable to open" << filename;
        return QJSValue(); // TODO: Exception
    }

    file.write(content.toUtf8());

    return QJSValue();
}
