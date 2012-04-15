
#include <QString>
#include <QElapsedTimer>
#include <QFile>
#include <QDebug>
#include <QStringList>
#include <QMutexLocker>

#include "troikaformats/archive.h"

class MethodTimer {
public:
    MethodTimer(const QString &msg) : message(msg) {
        timer.start();
    }
    ~MethodTimer() {
        qDebug() << qPrintable(message) << timer.elapsed() << "ms";
    }

private:
    QString message;
    QElapsedTimer timer;
};

class WildcardMatcher {
public:
    WildcardMatcher(const QString &pattern) {
        mPrefix = pattern.section('*', 0, 0);
        mSuffix = pattern.section('*', 1);
    }

    bool operator()(const QString &path) const {
        return path.startsWith(mPrefix, Qt::CaseInsensitive)
                && path.endsWith(mSuffix, Qt::CaseInsensitive);
    }
private:
    QString mPrefix;
    QString mSuffix;
};

bool TroikaArchive::open(const QString &filename)
{
    MethodTimer timer("Opened archive " + filename + " in ");

    mFile.setFileName(filename);

    if (!mFile.open(QFile::ReadOnly)) {
        mError = QString("Unable to open %1: %2")
                .arg(filename)
                .arg(mFile.errorString());
        mFile.close();
        return false;
    }

    // The last 4 byte of the file give the dictionary size
    if (!mFile.seek(mFile.size() - 4)) {
        mError = QString("Unable to seek to the size of the file table in %1: %2")
                .arg(filename)
                .arg(mFile.errorString());
        mFile.close();
        return false;
    }

    QDataStream stream(&mFile);
    stream.setByteOrder(QDataStream::LittleEndian);

    qint32 tableSize;
    stream >> tableSize;

    qDebug() << "Archive " << filename << " has a file table of size " << tableSize;

    // The table size actually includes the int we just read...
    if (!mFile.seek(mFile.size() - tableSize)) {
        mError = QString("Unable to seek to the file table in %1: %2")
                .arg(filename)
                .arg(mFile.errorString());
        mFile.close();
        return false;
    }

    QByteArray fileTable = mFile.read(tableSize);
    if (!parseFileTable(fileTable)) {
        mEntries.clear();
        mFile.close();
        return false;
    }

    return true;
}

void TroikaArchive::close()
{
    mFile.close();
}

QString TroikaArchive::error() const
{
    return mError;
}

QByteArray TroikaArchive::readFile(const QString &filename)
{
    const TroikaArchiveEntry *entry = findEntry(filename);

    if (entry) {
        return readEntry(entry);
    }

    return QByteArray();
}

const TroikaArchiveEntry *TroikaArchive::findEntry(QString path) const
{
    if (mEntries.isEmpty())
        return nullptr;

    if (path.contains('\\'))
        path.replace('\\', '/'); // Normalize the path...

    QString::SectionFlag flags = QString::SectionSkipEmpty;

    const TroikaArchiveEntry *current = mEntries.data();

    int section = 0;
    while (current) {
        QString pathPart = path.section('/', section, section, flags);
        section++;

        if (pathPart.isEmpty())
            break;

        if (section > 1)
            current = current->firstChild;

        for ( ; current ; current = current->nextSibling ) {
            if (!pathPart.compare(current->filename, Qt::CaseInsensitive)) {
                break;
            }
        }
    }

    return current;
}

QByteArray TroikaArchive::readEntry(const TroikaArchiveEntry *entry)
{
    Q_ASSERT(!entry->isDirectory());

    QMutexLocker locker(&mReadMutex);
    mFile.seek(entry->dataStart);

    // Decompress if needed
    if (entry->isCompressed()) {
        /* We need to prepend the uncompressed size so qUncompress will do its work... */
        QByteArray data(entry->compressedSize + sizeof(int), Qt::Uninitialized);
        QDataStream dataStream(&data, QIODevice::WriteOnly);
        dataStream << entry->uncompressedSize;
        mFile.read(data.data() + sizeof(int), entry->compressedSize);

        locker.unlock(); // Unlock before we uncompress to reduce locking...

        return qUncompress(data);
    } else {
        return mFile.read(entry->uncompressedSize);
    }
}

QByteArray TroikaArchive::readRawFile(const QString &filename, bool &compressed, int &uncompressedSize)
{
    const TroikaArchiveEntry *entry = findEntry(filename);

    if (entry && !entry->isDirectory()) {
        return readRawEntry(entry, compressed, uncompressedSize);
    }

    return QByteArray();
}

QByteArray TroikaArchive::readRawEntry(const TroikaArchiveEntry *entry, bool &compressed, int &uncompressedSize)
{
    Q_ASSERT(!entry->isDirectory());

    QMutexLocker locker(&mReadMutex);
    mFile.seek(entry->dataStart);

    // Decompress if needed
    compressed = entry->isCompressed();
    uncompressedSize = entry->uncompressedSize;
    return mFile.read(entry->isCompressed() ? entry->compressedSize : entry->uncompressedSize);
}

bool TroikaArchive::exists(const QString &filename) const
{
    return findEntry(filename) != nullptr;
}

TroikaArchive::EntryList TroikaArchive::listEntries(const QString &path, const QString &filter, bool directories) const
{
    const QRegExp SeparatorPattern("[/\\\\]+");
    QStringList pathParts = path.split(SeparatorPattern, QString::SkipEmptyParts);

    const TroikaArchiveEntry *current = mEntries.data();
    foreach (const QString &pathPart, pathParts) {
        while (current) {
            if (!pathPart.compare(current->filename, Qt::CaseInsensitive)) {
                current = current->firstChild;
                break;
            } else {
                current = current->nextSibling;
            }
        }

        if (!current) {
            qDebug() << "Unable to find" << path;
            break;
        }
    }

    TroikaArchive::EntryList results;

    while (current) {
        if (directories || !current->isDirectory())
            results.append(current);
        current = current->nextSibling;
    }

    return results;
}

TroikaArchive::EntryList TroikaArchive::listAllFiles(const QString &filenameFilter) const
{
    TroikaArchive::EntryList result;

    WildcardMatcher matcher(filenameFilter);

    for (int i = 0; i < mEntries.size(); ++i) {
        const TroikaArchiveEntry *entry = mEntries.data() + i;
        if (entry->isDirectory())
            continue;

        if (matcher(entry->filename))
            result.append(entry);
    }

    return result;
}

bool TroikaArchive::parseFileTable(const QByteArray &data)
{
    QDataStream stream(data);
    stream.setByteOrder(QDataStream::LittleEndian);

    uint fileCount;
    stream >> fileCount;

    qDebug() << "Archive file table contains " << fileCount << " entries.";

    mEntries.resize(fileCount);

    for (uint i = 0; i < fileCount; ++i) {
        if (!readFileTableEntry(stream, mEntries[i])) {
            return false;
        }
    }

    return true;
}

bool TroikaArchive::readFileTableEntry(QDataStream &stream, TroikaArchiveEntry &entry)
{
    QByteArray filename;
    qint32 unknown, parentDirId, firstChildId, nextSiblingId;

    // Read all data for entry
    stream >> filename >> unknown >> (qint32&)entry.type >> entry.uncompressedSize >> entry.compressedSize >> entry.dataStart
            >> parentDirId >> firstChildId >> nextSiblingId;

    // The null-byte may be included and needs to be stripped
    if (filename.endsWith('\0')) {
        filename.chop(1);
    }

    // Convert filename to QString
    entry.filename = filename;

    // Convert id's to pointers for faster traversal
    entry.parent = (parentDirId != -1) ? (mEntries.data() + parentDirId) : NULL;
    entry.nextSibling = (nextSiblingId != -1) ? (mEntries.data() + nextSiblingId) : NULL;
    entry.firstChild = (firstChildId != -1) ? (mEntries.data() + firstChildId) : NULL;

    Q_ASSERT(entry.isDirectory() || entry.firstChild == NULL); // Files mustn't have children

    return true;
}
