#ifndef TROIKAARCHIVE_H
#define TROIKAARCHIVE_H

#include <QVector>
#include <QList>
#include <QFile>
#include <QByteArray>
#include <QMutex>

#include "archiveentry.h"

class TroikaArchive
{
public:

    typedef QList<const TroikaArchiveEntry*> EntryList;

    bool open(const QString &filename);

    void close();

    /**
      Returns the last error, i.e. if open fails.
      */
    QString error() const;

    /**
      Reads the entire content of a file from the archive.
      */
    QByteArray readFile(const QString &filename);

    /**
      Reads the entire content of a file from the archive.
      */
    QByteArray readEntry(const TroikaArchiveEntry *entry);

    /**
      Reads a potentially compressed file from the archive but does not
      decompress it.
      */
    QByteArray readRawFile(const QString &filename, bool &compressed, int &uncompressedSize);

    /**
      Reads a potentially compressed file from the archive but does not
      decompress it.
      */
    QByteArray readRawEntry(const TroikaArchiveEntry *entry, bool &compressed, int &uncompressedSize);

    /**
      Checks that a given file exists within the archive.
      */
    bool exists(const QString &filename) const;

    /**
      Lists files within the archive, using the given path as the starting point.
      */
    EntryList listEntries(const QString &path, const QString &filter = "*", bool directories = false) const;

    /**
      Returns a list of all files in the archive, filtered using the given filename filter.
      */
    EntryList listAllFiles(const QString &filenameFilter) const;

    /**
      Tries to find a single entry based on its filename.
      */
    const TroikaArchiveEntry *findEntry(QString path) const;

private:
    QFile mFile;
    QVector<TroikaArchiveEntry> mEntries;
    QString mError;
    QMutex mReadMutex;

    bool parseFileTable(const QByteArray &data);
    bool readFileTableEntry(QDataStream &stream, TroikaArchiveEntry &entry);
};

#endif // TROIKAARCHIVE_H
