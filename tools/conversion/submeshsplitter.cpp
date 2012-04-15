
#include <QSet>

#include "submeshsplitter.h"

#include "troikaformats/model.h"
#include "troikaformats/skeleton.h"

SubMeshSplitter::SubMeshSplitter(QObject *parent) :
    QObject(parent)
{
}

/**
 * @brief Tries to find a closed subset of bones and mesh data to reduce the number of bones in a model to less than 256.
 * @param meshModel The mesh model that is to be split.
 * @param skeleton The skeleton.
 */
void SubMeshSplitter::findSubMeshes(const Troika::MeshModel *meshModel, const Troika::Skeleton *skeleton)
{

    /*
     * The most obvious split would be between face groups, so we try that first.
     */
    foreach (const QSharedPointer<Troika::FaceGroup> &faceGroup, meshModel->faceGroups()) {
        QSet<int> bones;

        foreach (const Troika::Face &face, faceGroup->faces()) {
            for (int i = 0; i < 3; ++i) {
                const Troika::Vertex &vertex = meshModel->vertices()[face.vertices[i]];
                for (int j = 0; j < vertex.attachmentCount; ++j) {
                    bones.insert(vertex.attachmentBone[j]);

                    // Add all parent bones as well

                }
            }
        }

        qDebug() << "Uses " << bones.size();
    }

}
