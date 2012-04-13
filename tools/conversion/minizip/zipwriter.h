#ifndef ZIPWRITER_H
#define ZIPWRITER_H

#include "minizipglobal.h"

#include <QString>
#include <QScopedPointer>

class ZipWriterData;

class ZipWriter
{
public:
    ZipWriter(const QString &filename);
    ~ZipWriter();

    bool addFile(const QString &filename, const QByteArray &data, int compressionLevel);
    void close();
private:
    QScopedPointer<ZipWriterData> d_ptr;
};

#endif // ZIPWRITER_H
