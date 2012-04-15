#ifndef SKMREADER_H
#define SKMREADER_H

#include <QString>
#include <QSharedPointer>

namespace Troika
{

    class VirtualFileSystem;
    class Materials;
    class MeshModel;
    class SkmReaderData;

    class SkmReader
    {
    public:
        SkmReader(const QString &filename, const QByteArray &data);
        ~SkmReader();

        MeshModel *get();

    private:
        QScopedPointer<SkmReaderData> d_ptr;
    };

}

#endif // SKMREADER_H
