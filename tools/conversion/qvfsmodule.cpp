
#include <QDir>
#include <QDebug>
#include <QStringList>
#include <QJSEngine>

#include "qvfsmodule.h"
#include "troikaformats/gamepathguesser.h"
#include "qbuffermodule.h"

#include "troikaformats/archive.h"
#include "qjsexceptionutils.h"

QVfsModule::QVfsModule(QCommonJSModule *commonJsModule) :
    QObject(commonJsModule), mCommonJsModule(commonJsModule), mEngine(commonJsModule->engine())
{
}

QVfsModule::~QVfsModule()
{
    clear();
}

void QVfsModule::clear()
{
    foreach (TroikaArchive *archive, mArchives) {
        delete archive;
    }
    mArchives.clear();
    mDirectories.clear();
}

QJSValue QVfsModule::guessGamePath() const
{
    QString gamePath = GamePathGuesser::guessGamePath();

    if (gamePath.isNull()) {
        return QJSValue();
    } else {
        return QJSValue(gamePath);
    }
}

QJSValue QVfsModule::validateGamePath(const QString &gamePath)
{
    QJSValue result = mEngine->newObject();

    if (!QDir(gamePath).exists()) {
        result.setProperty("valid", false);
        result.setProperty("reason", "directory_does_not_exist");
        return result;
    }

    QString basePath = gamePath;
    if (!basePath.endsWith(QDir::separator())) {
        basePath.append(QDir::separator());
    }

    QStringList missingFiles;

    for (int i = 1; i <= 4; ++i) {
        QString filename = QString("ToEE%2.dat").arg(i);

        if (!QFile::exists(basePath + filename)) {
            missingFiles << filename;
        }
    }

    QString modulePath = QString("modules%2ToEE.dat").arg(QDir::separator());
    if (!QFile::exists(basePath + modulePath)) {
        missingFiles << modulePath;
    }

    if (!missingFiles.isEmpty()) {
        result.setProperty("valid", false);
        result.setProperty("reason", "missing_files");
        result.setProperty("missingFiles", mEngine->toScriptValue(missingFiles));
        return result;
    }

    result.setProperty("valid", true);
    return result;
}

void QVfsModule::addDefaultArchives(const QString &basePath)
{
    // Add base archives
    for (int i = 1; i <= 4; ++i) {
        QString archivePath = QString("%1ToEE%2.dat").arg(basePath).arg(i);

        if (QFile::exists(archivePath)) {
            addArchive(archivePath);
        }
    }

    QString modulePath = QString("%1modules%2ToEE.dat").arg(basePath).arg(QDir::separator());
    addArchive(modulePath);
}

void QVfsModule::addArchive(const QString &archivePath)
{
    qDebug("Adding archive %s", qPrintable(archivePath));

    TroikaArchive *archive = new TroikaArchive;
    if (!archive->open(archivePath)) {
        qDebug() << "Cannot open" << archivePath << ":" << archive->error();
        delete archive;
        return;
    }

    mArchives.append(archive);
}

void QVfsModule::addOverrideDirectory(const QString &directory)
{

}

QJSValue QVfsModule::readFileAsString(const QString &path)
{
    foreach (TroikaArchive *archive, mArchives) {
        QByteArray content = archive->readFile(path);
        if (!content.isNull()) {
            return QString::fromLocal8Bit(content, content.size());
        }
    }

    return QJSValue();
}

QJSValue QVfsModule::readFile(const QString &path)
{
    foreach (TroikaArchive *archive, mArchives) {
        QByteArray content = archive->readFile(path);
        if (!content.isNull()) {
            return QBufferModule::newBuffer(mCommonJsModule, content);
        }
    }

    return QJSValue();
}

QJSValue QVfsModule::readFileRaw(const QString &path)
{
    foreach (TroikaArchive *archive, mArchives) {
        bool compressed = false;
        int uncompressedSize = 0;
        QByteArray content = archive->readRawFile(path, compressed, uncompressedSize);
        if (!content.isNull()) {
            QJSValue buffer = QBufferModule::newBuffer(mCommonJsModule, content);

            QJSValue result = mEngine->newObject();
            result.setProperty("compressed", compressed);
            result.setProperty("buffer", buffer);
            result.setProperty("uncompressedSize", uncompressedSize);
            return result;
        }
    }

    return QJSExceptionUtils::newError(mEngine, "File not found: " + path);

}

QJSValue QVfsModule::listAllFiles(const QString &filenameFilter) const
{

    QSet<QString> paths;

    foreach (TroikaArchive *archive, mArchives) {
        auto entries = archive->listAllFiles(filenameFilter);
        foreach (auto entry, entries) {
            paths.insert(entry->fullPath());
        }
    }

    QJSValue result = mEngine->newArray(paths.size());
    int offset = 0;
    foreach (const QString &path, paths) {
        result.setProperty(offset++, QJSValue(path));
    }

    return result;

}

QJSValue QVfsModule::listDirectory(const QString &directory) const
{

    bool found = false;
    QSet<QString> paths;

    QString basePath = directory;
    if (!basePath.endsWith(QDir::separator()))
        basePath.append(QDir::separator());

    foreach (TroikaArchive *archive, mArchives) {
        auto entry = archive->findEntry(directory);
        if (entry) {
            found = true;

            for (auto child = entry->firstChild; child; child = child->nextSibling) {
                if (!child->isDirectory()) {
                    paths.insert(basePath + child->filename);
                }
            }
        }
    }

    // None of the archives contained the directory.
    if (!found) {
        return QJSExceptionUtils::newError(mEngine, QString("Could not find directory: %1").arg(directory));
    }

    QJSValue result = mEngine->newArray(paths.size());
    int offset = 0;
    foreach (const QString &path, paths) {
        result.setProperty(offset++, QJSValue(path));
    }

    return result;

}
