
#include <Ogre.h>
#include <OgreMeshSerializer.h>
#include <OgreSkeletonSerializer.h>
#include <OgreDefaultHardwareBufferManager.h>
#include <OgreProgressiveMesh.h>
#include <OgreOptimisedUtil.h>

#include <QtGui/QVector3D>
#include <QtGui/QVector4D>
#include <QtCore/QScopedArrayPointer>
#include <QtCore/QVector>
#include <QtCore/QTemporaryFile>

#include "ogresystems.h"
#include "qjsexceptionutils.h"
#include "clippingconverter.h"
#include "qbuffermodule.h"
#include "qcommonjsmodule.h"

using Ogre::MeshPtr;
using Ogre::Mesh;

static MeshPtr importObject(QDataStream &stream);

ClippingConverter::ClippingConverter(QCommonJSModule *commonJs, OgreSystems *ogreSystems)
    : QObject(commonJs), mOgreSystems(ogreSystems), mCommonJs(commonJs)
{
}

QJSValue ClippingConverter::convert(const QJSValue &clippingDataJs)
{
    QByteArray clippingData = QBufferModule::getData(clippingDataJs);

    QDataStream stream(clippingData);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream.setByteOrder(QDataStream::LittleEndian);

    int objectCount, instanceCount;
    stream >> objectCount >> instanceCount;

    QJSValue objects = mCommonJs->engine()->newArray(objectCount);

    for (int i = 0; i < objectCount; ++i) {
        // Serialize to a temporary file, then read it in again
        QTemporaryFile tempFile;
        if (!tempFile.open()) {
            QString message = QString("Unable to open the temporary file: %1").arg(tempFile.fileName());
            return QJSExceptionUtils::newError(mCommonJs->engine(), message);
        }

        MeshPtr mesh = importObject(stream);
        mOgreSystems->meshSerializer->exportMesh(mesh.getPointer(), tempFile.fileName().toStdString());

        QByteArray meshData = tempFile.readAll();

    }

    QJSValue instances = mCommonJs->engine()->newArray(instanceCount);

    for (int i = 0; i < instanceCount; ++i) {
        QVector4D position;
        int index;

        stream >> position >> index;

        QJSValue jsPosition = mCommonJs->engine()->newArray(3);
        jsPosition.setProperty(0, position.x());
        jsPosition.setProperty(1, position.y());
        jsPosition.setProperty(2, position.z());

        QJSValue instance = mCommonJs->engine()->newObject();
        instance.setProperty("position", jsPosition);
        instance.setProperty("index", QJSValue(index));
        instances.setProperty(i, instance);
    }

    QJSValue result = mCommonJs->engine()->newObject();
    result.setProperty("instances", instances);
    result.setProperty("objects", objects);
    return result;
}

static MeshPtr importObject(QDataStream &stream)
{
    using namespace Ogre;

    QVector4D bbMin, bbMax;
    stream >> bbMin >> bbMax;

    float distance, distanceSquared; // Here's a bug for you: writes "double"'s instead of floats
    stream >> distanceSquared >> distance;

    MeshPtr ogreMesh = MeshManager::getSingleton().createManual("conversion",
                                                               ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

    int vertexCount, indexCount;
    stream >> vertexCount >> indexCount;

    VertexData *vertexData = new VertexData();
    ogreMesh->sharedVertexData = vertexData;

    LogManager::getSingleton().logMessage("Reading geometry...");
    VertexDeclaration* decl = vertexData->vertexDeclaration;
    VertexBufferBinding* bind = vertexData->vertexBufferBinding;
    unsigned short bufferId = 0;

    // Information for calculating bounds
    Vector3 min = Vector3::ZERO, max = Vector3::UNIT_SCALE, pos = Vector3::ZERO;
    Real maxSquaredRadius = -1;
    bool firstVertex = true;

    /*
      Create a vertex definition for our buffer
      */
    size_t offset = 0;

    const VertexElement &positionElement = decl->addElement(bufferId, offset, VET_FLOAT3, VES_POSITION);
    offset += VertexElement::getTypeSize(VET_FLOAT3);

    const VertexElement &normalElement = decl->addElement(bufferId, offset, VET_FLOAT3, VES_NORMAL);
    offset += VertexElement::getTypeSize(VET_FLOAT3);

    // calculate how many vertexes there actually are
    vertexData->vertexCount = vertexCount;

    // Now create the vertex buffer
    HardwareVertexBufferSharedPtr vbuf = HardwareBufferManager::getSingleton().
            createVertexBuffer(offset, vertexData->vertexCount,
                               HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);

    // Bind it
    bind->setBinding(bufferId, vbuf);

    // Lock it
    unsigned char *pVert = static_cast<unsigned char*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));
    unsigned char *pVertStart = pVert;

    QVector<float> positions;
    positions.reserve(vertexCount * 3);

    // Iterate over all children (vertexbuffer entries)
    for (int i = 0; i < vertexCount; ++i) {
        float *pFloat;

        QVector4D vertex;
        stream >> vertex;
		vertex.setZ(vertex.z() * -1);

        /* Copy over the position */
        positionElement.baseVertexPointerToElement(pVert, &pFloat);
        *(pFloat++) = (float)vertex.x();
        *(pFloat++) = (float)vertex.y();
        *(pFloat++) = (float)vertex.z();

        positions.append(vertex.x());
        positions.append(vertex.y());
        positions.append(vertex.z());

        /* While we're at it, calculate the bounding sphere */
        pos.x = vertex.x();
        pos.y = vertex.y();
        pos.z = vertex.z();

        if (firstVertex) {
            min = max = pos;
            maxSquaredRadius = pos.squaredLength();
            firstVertex = false;
        } else {
            min.makeFloor(pos);
            max.makeCeil(pos);
            maxSquaredRadius = qMax(pos.squaredLength(), maxSquaredRadius);
        }

        pVert += vbuf->getVertexSize();
    }

    // Set bounds
    const AxisAlignedBox& currBox = ogreMesh->getBounds();
    Real currRadius = ogreMesh->getBoundingSphereRadius();
    if (currBox.isNull())
    {
        //do not pad the bounding box
        ogreMesh->_setBounds(AxisAlignedBox(min, max), false);
        ogreMesh->_setBoundingSphereRadius(Math::Sqrt(maxSquaredRadius));
    }
    else
    {
        AxisAlignedBox newBox(min, max);
        newBox.merge(currBox);
        //do not pad the bounding box
        ogreMesh->_setBounds(newBox, false);
        ogreMesh->_setBoundingSphereRadius(qMax(Math::Sqrt(maxSquaredRadius), currRadius));
    }

    /*
       Create faces
     */
    // All children should be submeshes
    SubMesh* sm = ogreMesh->createSubMesh();
    sm->setMaterialName("clippingMaterial");
    sm->operationType = RenderOperation::OT_TRIANGLE_LIST;
    sm->useSharedVertices = true;

    // tri list
    sm->indexData->indexCount = indexCount;

    // Allocate space
    HardwareIndexBufferSharedPtr ibuf = HardwareBufferManager::getSingleton().
            createIndexBuffer(
                HardwareIndexBuffer::IT_16BIT,
                sm->indexData->indexCount,
                HardwareBuffer::HBU_DYNAMIC,
                false);
    sm->indexData->indexBuffer = ibuf;

    unsigned short *pShort = static_cast<unsigned short*>(ibuf->lock(HardwareBuffer::HBL_DISCARD));

    QVector<EdgeData::Triangle> triangles(indexCount / 3);

    for (int i = 0; i < indexCount / 3; ++i) {
        quint16 i1, i2, i3;

        stream >> i1 >> i2 >> i3;
        *pShort++ = i1;
        *pShort++ = i2;
        *pShort++ = i3;

        triangles[i].vertIndex[0] = i1;
        triangles[i].vertIndex[1] = i2;
        triangles[i].vertIndex[2] = i3;

    }

    /* Recalculate the vertex normals */
    Vector4 *faceNormals = (Vector4*)_aligned_malloc(sizeof(Vector4) * triangles.size(), 16);

    OptimisedUtil *util = OptimisedUtil::getImplementation();
    util->calculateFaceNormals(positions.constData(),
                               triangles.data(),
                               faceNormals,
                               indexCount / 3);

	 // Iterate over all children (vertexbuffer entries)
	pVert = pVertStart;
    for (int i = 0; i < vertexCount; ++i) {
        float *pFloat;

		Vector3 normal = Vector3::ZERO;
		
		int count = 0;

		/* Search for all faces that use this vertex */
		for (int j = 0; j < triangles.size(); ++j) {
			if (triangles[j].vertIndex[0] == i 
				|| triangles[j].vertIndex[1] == i 
				|| triangles[j].vertIndex[2] == i) {
				normal.x += faceNormals[j].x / faceNormals[j].w;
				normal.y += faceNormals[j].y / faceNormals[j].w;
				normal.z += faceNormals[j].z / faceNormals[j].w;
				count++;
			}
		}

		normal.normalise();

        /* Copy over the position */
		normalElement.baseVertexPointerToElement(pVert, &pFloat);
        *(pFloat++) = normal.x;
        *(pFloat++) = normal.y;
        *(pFloat++) = normal.z;
		
        pVert += vbuf->getVertexSize();
    }

    _aligned_free(faceNormals);

    vbuf->unlock();
    ibuf->unlock();

    return ogreMesh;
}
