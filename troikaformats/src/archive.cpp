
#include <QString>
#include <QFile>

#include "troikaformats/archive.h"

class TroikaArchivePrivate {
public:
    TroikaArchivePrivate(TroikaArchive *troikaArchive);

    QFile file;
    TroikaArchive *q;
};

TroikaArchive::TroikaArchive() : d(new TroikaArchivePrivate(this))
{
}

TroikaArchive::~TroikaArchive()
{
    delete d;
}

bool TroikaArchive::open(const QString &filename)
{
    d->file.setFileName(filename);

    return d->file.open(QIODevice::ReadOnly);
}

void TroikaArchive::close()
{
    d->file.close();
}

TroikaArchivePrivate::TroikaArchivePrivate(TroikaArchive *troikaArchive) : q(troikaArchive)
{
}
