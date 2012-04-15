#ifndef TROIKAFORMATS_DAGREADER_H
#define TROIKAFORMATS_DAGREADER_H

#include <QString>
#include <QSharedPointer>
namespace Troika {

    class MeshModel;

    class DagReader {
    public:
        static MeshModel *read(const QByteArray &dagData);
    };

}

#endif // TROIKAFORMATS_DAGREADER_H
