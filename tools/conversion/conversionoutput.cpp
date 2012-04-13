
#include <QDebug>

#include "conversionoutput.h"

#include "qcommonjsmodule.h"
#include "qbuffermodule.h"
#include "qjsexceptionutils.h"

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
        QString message = QString("Unable to create %1").arg(filename);
        return QJSExceptionUtils::newError(mCommonJsModule->engine(), message);
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
        QString message = QString("Unable to create %1").arg(filename);
        return QJSExceptionUtils::newError(mCommonJsModule->engine(), message);
    }

    file.write(content.toUtf8());

    return QJSValue();
}


QJSValue ConversionOutput::openCategoryAsDirectory(const QString &category, const QString &baseDirectory)
{
    return QJSExceptionUtils::newError(mCommonJsModule->engine(), "Not implemented.");
}

QJSValue ConversionOutput::openCategoryAsArchive(const QString &category, const QString &archiveFilename)
{
    return QJSExceptionUtils::newError(mCommonJsModule->engine(), "Not implemented.");
}

QJSValue ConversionOutput::close()
{
    return QJSExceptionUtils::newError(mCommonJsModule->engine(), "Not implemented.");
}
