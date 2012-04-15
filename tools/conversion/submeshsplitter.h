#ifndef SUBMESHSPLITTER_H
#define SUBMESHSPLITTER_H

#include <QObject>

namespace Troika {
    class MeshModel;
    class Skeleton;
}

class SubMeshSplitter : public QObject
{
    Q_OBJECT
public:
    explicit SubMeshSplitter(QObject *parent = 0);
    
    void findSubMeshes(const Troika::MeshModel *meshModel, const Troika::Skeleton *skeleton);

signals:
    
public slots:
    
};

#endif // SUBMESHSPLITTER_H
