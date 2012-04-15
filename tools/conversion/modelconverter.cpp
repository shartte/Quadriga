
#include <Ogre.h>
#include <OgreMeshSerializer.h>
#include <OgreSkeletonSerializer.h>
#include <OgreDefaultHardwareBufferManager.h>
#include <OgreProgressiveMesh.h>

#include "modelconverter.h"
#include "submeshsplitter.h"

#include <QtCore/QTemporaryFile>

#include <troikaformats/model.h>
#include <troikaformats/skeleton.h>
#include <troikaformats/skmreader.h>

#include "ogresystems.h"
#include "qcommonjsmodule.h"
#include "qbuffermodule.h"
#include "qjsexceptionutils.h"

// Uncomment this line to have the converter flip the Z axis of all models
// #define FLIP_Z

ModelConverter::ModelConverter(QCommonJSModule *commonJs, OgreSystems *ogreSystems) : QObject(commonJs), mCommonJs(commonJs), mOgreSystems(ogreSystems)
{
}

QJSValue ModelConverter::convert(QJSValue meshDataJs, QJSValue skeletonDataJs, QJSValue materialNameCallback, QJSValue skeletonNameCallback)
{
    if (!QBufferModule::isBuffer(meshDataJs)) {
        return QJSExceptionUtils::newError(mCommonJs->engine(), "The first argument must be a buffer.");
    }
    if (!skeletonDataJs.isUndefined() && !skeletonDataJs.isNull() && !QBufferModule::isBuffer(meshDataJs)) {
        return QJSExceptionUtils::newError(mCommonJs->engine(), "The second argument must be null, undefined or a buffer.");
    }
    if (!materialNameCallback.isCallable() || !skeletonNameCallback.isCallable()) {
        return QJSExceptionUtils::newError(mCommonJs->engine(), "The third and fourth argument must be callbacks.");
    }

    QByteArray data = QBufferModule::getData(meshDataJs);

    /* Read the model file */
    Troika::SkmReader reader("input.skm", data);
    QScopedPointer<Troika::MeshModel> model(reader.get());

    if (QBufferModule::isBuffer(skeletonDataJs)) {
        data = QBufferModule::getData(skeletonDataJs);

        auto skeleton = new Troika::Skeleton(data, "input.ska");
        if (skeleton->bones().size() >= 256) {
            // Find smallest contiguous sets of bones and vertices
            SubMeshSplitter splitter;
            splitter.findSubMeshes(model.data(), skeleton);

            QString message = QString("Unable to convert a model with more than 256 bones (has %1).").arg(skeleton->bones().size());
            return QJSExceptionUtils::newError(mCommonJs->engine(), message);
        }

        model->setSkeleton(skeleton);
    }

    data = convertMesh(*model, materialNameCallback, skeletonNameCallback);

    QJSValue result = mCommonJs->engine()->newObject();
    result.setProperty("mesh", QBufferModule::newBuffer(mCommonJs, data));

    if (model->skeleton()) {
        data = convertSkeleton(*model);
        result.setProperty("skeleton", QBufferModule::newBuffer(mCommonJs, data));
    }

    return result;
}

QByteArray ModelConverter::convertMesh(const Troika::MeshModel &model, QJSValue materialNameCallback, QJSValue skeletonNameCallback)
{
    using namespace Ogre;
    MeshPtr newMesh = MeshManager::getSingleton().createManual("conversion",
                                                               ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    importMesh(newMesh.getPointer(), model, materialNameCallback, skeletonNameCallback);

    postprocessMesh(newMesh.getPointer());

    // Serialize to a temporary file, then read it in again
    QTemporaryFile tempFile;
    if (!tempFile.open()) {
        qWarning("Unable to open the temporary %s file.", qPrintable(tempFile.fileName()));
        return QByteArray();
    }

    qDebug("Using temporary file for Mesh serialization: %s", qPrintable(tempFile.fileName()));

    mOgreSystems->meshSerializer->exportMesh(newMesh.getPointer(), tempFile.fileName().toStdString());

    return tempFile.readAll();
}

QString ModelConverter::getSkeletonName(QJSValue skeletonNameCallback, const QString &filename)
{
    if (!skeletonNameCallback.isUndefined())
        return skeletonNameCallback.call(QJSValueList() << filename).toString();
    else
        return QString::null;
}

QString ModelConverter::getMaterialName(QJSValue materialNameCallback, const QString &filename)
{
    if (!materialNameCallback.isUndefined())
        return materialNameCallback.call(QJSValueList() << filename).toString();
    else
        return QString::null;
}

void ModelConverter::importMesh(Ogre::Mesh *ogreMesh, const Troika::MeshModel &troikaModel, QJSValue materialNameCallback, QJSValue skeletonNameCallback)
{
    using namespace Ogre;

    /* Generate shared geometry from vertices */
    importGeometry(ogreMesh, troikaModel);

    /* Generate submeshes from face groups */
    importSubmeshes(ogreMesh, troikaModel, materialNameCallback);

    /* Link to skeleton if there is one */
    if (troikaModel.skeleton()) {
        QString filename = getSkeletonName(skeletonNameCallback, troikaModel.skeleton()->filename());
        if (!filename.isEmpty())
            ogreMesh->setSkeletonName(filename.toStdString());
    }

    /* Import bone assignments from vertices */
    importBoneAssignments(ogreMesh, troikaModel);

    LogManager::getSingleton().logMessage("XMLMeshSerializer import successful.");
}

void ModelConverter::importGeometry(Ogre::Mesh *ogreMesh, const Troika::MeshModel &troikaModel)
{
    using namespace Ogre;

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

    const VertexElement &texElement = decl->addElement(bufferId, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);
    offset += VertexElement::getTypeSize(VET_FLOAT2);

    // calculate how many vertexes there actually are
    vertexData->vertexCount = troikaModel.vertices().size();

    // Now create the vertex buffer
    HardwareVertexBufferSharedPtr vbuf = HardwareBufferManager::getSingleton().
            createVertexBuffer(offset, vertexData->vertexCount,
                               HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);

    // Bind it
    bind->setBinding(bufferId, vbuf);

    // Lock it
    unsigned char *pVert = static_cast<unsigned char*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));

    // Iterate over all children (vertexbuffer entries)
    foreach (const Troika::Vertex &vertex, troikaModel.vertices()) {
        float *pFloat;

        /* Copy over the position */
        positionElement.baseVertexPointerToElement(pVert, &pFloat);
        *(pFloat++) = vertex.positionX;
        *(pFloat++) = vertex.positionY;
#if defined(FLIP_Z)
        *(pFloat++) = - vertex.positionZ;
#else
        *(pFloat++) = vertex.positionZ;
#endif

        /* Copy over the normal */
        normalElement.baseVertexPointerToElement(pVert, &pFloat);
        *(pFloat++) = vertex.normalX;
        *(pFloat++) = vertex.normalY;
#if defined(FLIP_Z)
        *(pFloat++) = - vertex.normalZ;
#else
        *(pFloat++) = vertex.normalZ;
#endif

        /* Copy over the texture coordinate */
        texElement.baseVertexPointerToElement(pVert, &pFloat);
        *(pFloat++) = vertex.texCoordX;
        *(pFloat++) = 1 - vertex.texCoordY; // V is flipped

        pVert += vbuf->getVertexSize();

        /* While we're at it, calculate the bounding sphere */
        pos.x = vertex.positionX;
        pos.y = vertex.positionY;
#if defined(FLIP_Z)
        pos.z = - vertex.positionZ;
#else
        pos.z = vertex.positionZ;
#endif

        if (firstVertex) {
            min = max = pos;
            maxSquaredRadius = pos.squaredLength();
            firstVertex = false;
        } else {
            min.makeFloor(pos);
            max.makeCeil(pos);
            maxSquaredRadius = qMax(pos.squaredLength(), maxSquaredRadius);
        }
    }

    vbuf->unlock();

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

    LogManager::getSingleton().logMessage("Geometry done...");
}

void ModelConverter::importSubmeshes(Ogre::Mesh *ogreMesh, const Troika::MeshModel &troikaModel, QJSValue materialNameCallback)
{
    using namespace Ogre;

    LogManager::getSingleton().logMessage("Reading submeshes...");

    foreach (const QSharedPointer<Troika::FaceGroup> &faceGroup, troikaModel.faceGroups()) {

        // All children should be submeshes
        SubMesh* sm = ogreMesh->createSubMesh();

        QString materialName = getMaterialName(materialNameCallback, faceGroup->material());
        if (!materialName.isEmpty())
            sm->setMaterialName(materialName.toStdString());

        sm->operationType = RenderOperation::OT_TRIANGLE_LIST;

        sm->useSharedVertices = true;

        int count = faceGroup->faces().size();

        // tri list
        sm->indexData->indexCount = count * 3;

        // Allocate space
        HardwareIndexBufferSharedPtr ibuf = HardwareBufferManager::getSingleton().
                createIndexBuffer(
                    HardwareIndexBuffer::IT_16BIT,
                    sm->indexData->indexCount,
                    HardwareBuffer::HBU_DYNAMIC,
                    false);
        sm->indexData->indexBuffer = ibuf;

        unsigned short *pShort = static_cast<unsigned short*>(ibuf->lock(HardwareBuffer::HBL_DISCARD));

        foreach (const Troika::Face &face, faceGroup->faces()) {
#if defined(FLIP_Z)
            /* Due to flipping the Z axis, we need to invert the winding here */
            *pShort++ = face.vertices[2];
            *pShort++ = face.vertices[1];
            *pShort++ = face.vertices[0];
#else
            *pShort++ = face.vertices[0];
            *pShort++ = face.vertices[1];
            *pShort++ = face.vertices[2];
#endif
        }

        ibuf->unlock();
    }

    LogManager::getSingleton().logMessage("Submeshes done.");
}

static void createBoneIdMapping(QVector<int> &boneIdMapping, const Troika::MeshModel &model)
{
    /* Build a hash over the names */
    QHash<QByteArray,int> boneNameMap;

    const Troika::Skeleton *skeleton = model.skeleton();
    foreach (const Troika::Bone &bone, skeleton->bones()) {
        boneNameMap[bone.name] = bone.id;
    }

    for (int i = 0; i < model.bindingPoseBones().size(); ++i) {

        const Troika::BindingPoseBone &bone = model.bindingPoseBones().at(i);

        // First try: Try by name
        auto it = boneNameMap.find(bone.name);

        if (boneNameMap.end() != it) {
            boneIdMapping[i] = it.value();
            continue; // The best kind of mapping
        }

        QByteArray sameIndexName = "<Unknown>";

        if (i < skeleton->bones().size())
            sameIndexName = skeleton->bones()[i].name;

        // We couldn't find a bone that matches.
        qDebug("Couldn't find a matching bone for '%s' in the skeleton. Bone with same index: '%s'",
               bone.name.constData(),
               sameIndexName.constData());
    }
}

static bool assignmentLessThan(const QPair<int, float> &a, const QPair<int, float> &b)
{
    return b.first < a.first;
}

void ModelConverter::importBoneAssignments(Ogre::Mesh *ogreMesh, const Troika::MeshModel &troikaModel)
{
    using Ogre::VertexBoneAssignment;
    using Ogre::LogManager;

	if (!troikaModel.skeleton()) {
		return;
	}

    LogManager::getSingleton().logMessage("Reading bone assignments...");

    /*
        Sadly we have to map the bone assignments to bones from the skeleton.
        While in ToEE, every mesh can have its own binding pose, in Ogre, the
        binding pose is solely defined by the skeleton. To make this worse,
        the mesh in ToEE does not always have the same bones as the skeleton.

        Here we create a mapping between the bone ids found in the meshes binding
        pose and the bone ids that will be found in the corresponding skeleton.
      */
    QVector<int> boneIdMapping(troikaModel.bindingPoseBones().size(), -1);

    createBoneIdMapping(boneIdMapping, troikaModel);

    /*
      TODO: Re-Weight the assignments to 4 weights if there are 5 or 6.
      */
    int vertexCount = troikaModel.vertices().size();

    QVector<QPair<int, float> > assignments;
    assignments.reserve(6);

    for (int i = 0; i < vertexCount; ++i) {
        const Troika::Vertex &vertex = troikaModel.vertices().at(i);


        if (vertex.attachmentCount > 4) {
            qDebug("Re-weighting vertex bone assignment from %d.", vertex.attachmentCount);

            float totalWeight = 0;

            assignments.resize(vertex.attachmentCount);
            for (int j = 0; j < vertex.attachmentCount; ++j) {
                assignments[j].first = vertex.attachmentBone[j];
                assignments[j].second = vertex.attachmentWeight[j];
                totalWeight += vertex.attachmentWeight[j];
            }

            // NOTE: assignmentLessThan is reversed
            qSort(assignments.begin(), assignments.end(), assignmentLessThan);

            // Re-weight
            float weightFactor = totalWeight / (assignments[0].second
                    + assignments[1].second
                    + assignments[2].second
                    + assignments[3].second);
            assignments[0].second *= weightFactor;
            assignments[1].second *= weightFactor;
            assignments[2].second *= weightFactor;
            assignments[3].second *= weightFactor;

            for (int j = 0; j < 4; ++j) {
                VertexBoneAssignment vba;
                vba.vertexIndex = i;
                vba.boneIndex = boneIdMapping[assignments[j].first];
                vba.weight = assignments[j].second;
                ogreMesh->addBoneAssignment(vba);
            }

        } else {
            for (int j = 0; j < vertex.attachmentCount; ++j) {
                VertexBoneAssignment vba;
                vba.vertexIndex = i;
                vba.boneIndex = boneIdMapping[vertex.attachmentBone[j]];
                vba.weight = vertex.attachmentWeight[j];
                ogreMesh->addBoneAssignment(vba);
            }
        }

    }

    LogManager::getSingleton().logMessage("Bone assignments done.");
}

void ModelConverter::postprocessMesh(Ogre::Mesh *mesh)
{
    using namespace Ogre;

    mesh->buildEdgeList(); // For stencil shadows

    qDebug("Reorganizing vertex buffers.");
    // Automatic
    VertexDeclaration* newDcl =
            mesh->sharedVertexData->vertexDeclaration->getAutoOrganisedDeclaration(
            mesh->hasSkeleton(), mesh->hasVertexAnimation());

    if (*newDcl != *(mesh->sharedVertexData->vertexDeclaration))
    {
        // Usages don't matter here since we're onlly exporting
        BufferUsageList bufferUsages;
        for (size_t u = 0; u <= newDcl->getMaxSource(); ++u)
            bufferUsages.push_back(HardwareBuffer::HBU_STATIC_WRITE_ONLY);
        mesh->sharedVertexData->reorganiseBuffers(newDcl, bufferUsages);
    }
}

QByteArray ModelConverter::convertSkeleton(const Troika::MeshModel &model)
{
    using namespace Ogre;

    // TODO: Check if this skeleton needs to be manually unloaded

    SkeletonPtr newSkel = SkeletonManager::getSingleton().create("conversion",
                                                                 ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

    qDebug("Importing skeleton...");
    importSkeleton(newSkel.getPointer(), model);

    qDebug("Optimizing animations...");
    newSkel->optimiseAllAnimations();

    // Serialize to a temporary file, then read it in again
    QTemporaryFile tempFile;
    if (!tempFile.open()) {
        qWarning("Unable to open the temporary %s file.", qPrintable(tempFile.fileName()));
        return QByteArray();
    }

    qDebug("Using temporary file for Skeleton serialization: %s", qPrintable(tempFile.fileName()));

    try {
        mOgreSystems->skeletonSerializer->exportSkeleton(newSkel.getPointer(), tempFile.fileName().toStdString());
    } catch (InvalidParametersException e) {
        qFatal("Unable to export Skeleton: %s", e.getFullDescription().c_str());
    }

    return tempFile.readAll();
}

void ModelConverter::importSkeleton(Ogre::Skeleton *ogreSkeleton, const Troika::MeshModel &troikaModel)
{
    importBones(ogreSkeleton, troikaModel);
    importAnimations(ogreSkeleton, troikaModel);
}

static void createBoneHierarchy(Ogre::Skeleton *ogreSkeleton, Ogre::Bone *ogreBone, const Troika::Bone &bone)
{
    /* Create the bone hierarchy in the same go */
    if (bone.parentId != -1) {
        /* Otherwise we'd need to use a two-pass algorithm */
        Q_ASSERT(bone.parentId < bone.id);

        Ogre::Bone *ogreParentBone = ogreSkeleton->getBone(bone.parentId);
        Q_ASSERT(ogreParentBone);

        ogreParentBone->addChild(ogreBone);
    }
}

static void createBindingPose(const Troika::MeshModel &model, const Troika::Bone &bone, Ogre::Bone *ogreBone)
{
    using namespace Ogre;

    /*
      Problem here: Animations are in terms of the skeleton's bones, while the
      bones here require the binding pose bones. In order to make this work, we
      have to maintain the indices used by the binding pose. As such, we'll create the bones
      from the binding pose.
     */

    // Find a related binding pose bone (TODO: Introduce a QHash here)
    foreach (const Troika::BindingPoseBone &bindingPoseBone, model.bindingPoseBones()) {
        if (bindingPoseBone.name == bone.name) {

            QQuaternion rotation = bindingPoseBone.rotation;

#if defined(FLIP_Z)
            ogreBone->setOrientation(rotation.scalar(),
                                     - rotation.x(),
                                     - rotation.y(),
                                     rotation.z());

            ogreBone->setPosition(bindingPoseBone.position.x(),
                                  bindingPoseBone.position.y(),
                                  - bindingPoseBone.position.z());
#else
            ogreBone->setOrientation(rotation.scalar(),
                                     rotation.x(),
                                     rotation.y(),
                                     rotation.z());

            ogreBone->setPosition(bindingPoseBone.position.x(),
                                  bindingPoseBone.position.y(),
                                  bindingPoseBone.position.z());
#endif

            ogreBone->setScale(bindingPoseBone.scale.x(),
                               bindingPoseBone.scale.y(),
                               bindingPoseBone.scale.z());

            return;
        }
    }

    qWarning("Couldn't find a bone in the binding pose that matches %s. Falling back to rest pose.", bone.name.constData());
    ogreBone->setScale(bone.scale.x(), bone.scale.y(), bone.scale.z());
#if defined(FLIP_Z)
    ogreBone->setPosition(bone.translation.x(), bone.translation.y(), - bone.translation.z());
    ogreBone->setOrientation(bone.rotation.scalar(),
                             - bone.rotation.x(),
                             - bone.rotation.y(),
                             bone.rotation.z());
#else
    ogreBone->setPosition(bone.translation.x(), bone.translation.y(), bone.translation.z());
    ogreBone->setOrientation(bone.rotation.scalar(),
                             bone.rotation.x(),
                             bone.rotation.y(),
                             bone.rotation.z());
#endif
}

void ModelConverter::importBones(Ogre::Skeleton *ogreSkeleton, const Troika::MeshModel &troikaModel)
{
    const Troika::Skeleton *skeleton = troikaModel.skeleton();

    qDebug("Creating %d bones...", skeleton->bones().size());

    foreach (const Troika::Bone &bone, skeleton->bones()) {
        Ogre::Bone *ogreBone = ogreSkeleton->createBone(bone.name.constData(), bone.id);

        createBoneHierarchy(ogreSkeleton, ogreBone, bone);

        /* Set binding pose attributes */
        createBindingPose(troikaModel, bone, ogreBone);
    }
}

Ogre::Quaternion toOgre(const QQuaternion &quat) {
#if defined(FLIP_Z)
    return Ogre::Quaternion(quat.scalar(), - quat.x(), - quat.y(), quat.z());
#else
    return Ogre::Quaternion(quat.scalar(), quat.x(), quat.y(), quat.z());
#endif
}

Ogre::Vector3 toOgre(const QVector3D &v, bool flipZ = false) {
#if defined(FLIP_Z)
    if (flipZ)
        return Ogre::Vector3(v.x(), v.y(), - v.z());
 #endif
    return Ogre::Vector3(v.x(), v.y(), v.z());
}

inline QQuaternion rebase(const QQuaternion &value, const QQuaternion &newBase)
{
    return newBase.conjugate() * value;
}

inline QVector3D rebaseTranslation(const QVector3D &value, const QVector3D &newBase)
{
    return value - newBase;
}

QVector3D rebaseScale(const QVector3D &value, const QVector3D &newBase)
{
    return QVector3D(value.x() / newBase.x(),
                     value.y() / newBase.y(),
                     value.z() / newBase.z());
}

struct BoneRestingState {
    QQuaternion rotation;
    QVector3D scale;
    QVector3D translation;
    bool fromBindingPose;
};

void writeRestingPoseAnimation(Ogre::Skeleton *ogreSkeleton,
                               const Troika::Skeleton *skeleton,
                               const QVector<BoneRestingState> &boneRestingStates)
{
    using namespace Ogre;

    /* Adds a default pose animation based on the skeleton's base state for bones */
    Animation *defaultPose = ogreSkeleton->createAnimation("defaultpose", 0.0f);
    defaultPose->setInterpolationMode(Animation::IM_LINEAR);

    foreach (const Troika::Bone &bone, skeleton->bones()) {
        const BoneRestingState &boneRestingState = boneRestingStates.at(bone.id);

        NodeAnimationTrack *track = defaultPose->createNodeTrack(bone.id, ogreSkeleton->getBone(bone.id));

        TransformKeyFrame *keyFrame = track->createNodeKeyFrame(0);
        keyFrame->setRotation(toOgre(rebase(bone.rotation, boneRestingState.rotation)));
        keyFrame->setScale(toOgre(rebaseScale(bone.scale, boneRestingState.scale)));
        keyFrame->setTranslate(toOgre(rebaseTranslation(bone.translation, boneRestingState.translation), true));
    }
}

inline float getKeyFrameTime(const Troika::Animation &animation, int keyFrame)
{
    if (animation.driveType() == Troika::Animation::Time)
        return keyFrame / animation.frameRate();

    /*
      Example for rotation based animation:
      - Rotation per second: 3.76 (Radians/Second)
      - FrameRate: 5 (Frames/Radians)
      - Frame: 5
      - Time: (1 / rps) *
      */

    return keyFrame * (1 / animation.frameRate()) * (1 / qAbs(animation.dps()));
}

struct DetachedKeyFrame {
    QQuaternion rotation;
    QVector3D translation;
    QVector3D scale;
    bool rotationSet;
    bool translationSet;
    bool scaleSet;

    DetachedKeyFrame() : rotationSet(false), translationSet(false), scaleSet(false)
    {
    }
};

static void fillSparseKeyframes(QMap<int, DetachedKeyFrame> &keyFrames);
static void parseKeyFrames(const Troika::Skeleton *skeleton,
                           const Troika::Animation &animation,
                           const QVector<BoneRestingState> &boneRestingState,
                           QVector<QMap<int,DetachedKeyFrame> > &keyFramesPerBone);

void ModelConverter::importAnimations(Ogre::Skeleton *ogreSkeleton, const Troika::MeshModel &troikaModel)
{

    using Ogre::Animation;
    using Ogre::TransformKeyFrame;
    using Ogre::NodeAnimationTrack;

    const Troika::Skeleton *skeleton = troikaModel.skeleton();

    /* Creating a mapping between skeleton bones and the corresponding bones in the binding pose? Annoying. */
    QVector<BoneRestingState> boneRestingStates(skeleton->bones().size());
    foreach (const Troika::Bone &bone, skeleton->bones()) {
        bool found = false;

        BoneRestingState &boneRestingState = boneRestingStates[bone.id];

        /* Find a matching bone */
        foreach (const Troika::BindingPoseBone &bindingBone, troikaModel.bindingPoseBones()) {
            if (bindingBone.name == bone.name) {
                boneRestingState.rotation = bindingBone.rotation;
                boneRestingState.translation = bindingBone.position;
                boneRestingState.scale = bindingBone.scale;
                found = true;
                break;
            }
        }

        boneRestingState.fromBindingPose = found;

        if (!found) {
            qDebug("Couldn't find a bone matching '%s' in the binding pose.", bone.name.constData());
            /* use the resting pose as the base state instead */
            boneRestingState.rotation = bone.rotation;
            boneRestingState.translation = bone.translation;
            boneRestingState.scale = bone.scale;
        }
    }

    writeRestingPoseAnimation(ogreSkeleton, skeleton, boneRestingStates);

    QVector<QMap<int,DetachedKeyFrame> > keyFramesPerBone(skeleton->bones().size()); // LUT for frames
    QHash<int, QByteArray> animationsByDataStart; // Contains the start offset of all animations that have been written and their name
    QHash<QByteArray, QByteArray> animationAliases;

    /* Export all other animations */
    foreach (const Troika::Animation &animation, skeleton->animations()) {

        int keyFrameStart = animation.keyFramesDataStart();

        if (animationsByDataStart.contains(keyFrameStart)) {
            animationAliases[animation.name()] = animationsByDataStart[keyFrameStart];
            continue; // Skip animations
        }

        animationsByDataStart[keyFrameStart] = animation.name();

        float length = getKeyFrameTime(animation, animation.frames() - 1);
        Animation *ogreAnim = ogreSkeleton->createAnimation(animation.name().constData(), length);

        /*
            Ogre assumes (at least before 1.8) that the binding pose is the default pose.
            In ToEE, we have a resting pose too, to which all animations are relative.
            We need to write out the difference between the binding and resting pose as
            the first frame of each animation, unless the bones are affected by the animation.
        */
        foreach (const Troika::Bone &bone, skeleton->bones()) {
            const BoneRestingState &boneRestingState = boneRestingStates.at(bone.id);

            /* Always reset bone state */
            keyFramesPerBone[bone.id].clear();

            DetachedKeyFrame keyFrame;
            keyFrame.rotation = rebase(bone.rotation, boneRestingState.rotation);
            keyFrame.rotationSet = true;
            keyFrame.scale = rebaseScale(bone.scale, boneRestingState.scale);
            keyFrame.scaleSet = true;
            keyFrame.translation = rebaseTranslation(bone.translation, boneRestingState.translation);
            keyFrame.translationSet = true;
            keyFramesPerBone[bone.id][0] = keyFrame;
        }

        /*
          TODO: Write the actual key frame values...
         */
        parseKeyFrames(skeleton, animation, boneRestingStates, keyFramesPerBone);

        /*
          Now we have to convert from a sparse keyframe array into actual keyframes. But before we can do that,
          we need to fill "gaps" of undefined translations, rotations and scale in the keyframes.
         */
        foreach (const Troika::Bone &bone, skeleton->bones()) {
            QMap<int, DetachedKeyFrame> &keyFrames = keyFramesPerBone[bone.id];

            if (keyFrames.isEmpty())
                continue; // Nothing to proces for this bone

            fillSparseKeyframes(keyFrames);

            /* Check if there's a keyframe coinciding with the duration of the animation and create one if necessary */
            // But only if the animation is not intended to loop (and thus bounce back to the initial state anyway
            if (!keyFrames.contains(animation.frames()) && keyFrames.size() > 1) {
                    /* Get the "last" keyframe" */
                    // qDebug("Animation '%s' has no 'last' frame. Cloning second-to-last.", animation.name().constData());
                    if (animation.loopable()) {
                        DetachedKeyFrame firstKeyFrame = keyFrames[keyFrames.keys().first()];
                        keyFrames[animation.frames()] = firstKeyFrame;
                    } else {
                        DetachedKeyFrame lastKeyFrame = keyFrames[keyFrames.keys().last()];
                        keyFrames[animation.frames()] = lastKeyFrame;
                    }
            }

            /* Create track & dump the info */
            Ogre::NodeAnimationTrack *track = ogreAnim->createNodeTrack(bone.id, ogreSkeleton->getBone(bone.id));

            QMap<int,DetachedKeyFrame>::const_iterator it = keyFrames.begin();
            for (; it != keyFrames.end(); ++it) {
                float time = getKeyFrameTime(animation, it.key());

                Ogre::TransformKeyFrame *ogreKeyFrame = track->createNodeKeyFrame(time);

                const DetachedKeyFrame &keyFrame = it.value();

                if (keyFrame.rotationSet)
                    ogreKeyFrame->setRotation(toOgre(keyFrame.rotation));
                if (keyFrame.scaleSet)
                    ogreKeyFrame->setScale(toOgre(keyFrame.scale));
                if (keyFrame.translationSet)
                    ogreKeyFrame->setTranslate(toOgre(keyFrame.translation, true));

            }
        }

    }

}

static void copyBoneStateToFrames(Troika::AnimationStream *stream,
                                  const QVector<BoneRestingState> &boneRestingState,
                                  QVector<QMap<int,DetachedKeyFrame> > &keyFramesPerBone)
{
    foreach (const Troika::AnimationBoneState &state, stream->boneStates()) {

        /* Base-State for this bone may either come from the rest-pose or the binding pose */
        const BoneRestingState &restingState = boneRestingState[state.boneId];

        QMap<int, DetachedKeyFrame> &frames = keyFramesPerBone[state.boneId];

        DetachedKeyFrame &scaleFrame = frames[state.scaleFrame];
        scaleFrame.scale = rebaseScale(state.scale, restingState.scale);
        scaleFrame.scaleSet = true;

        DetachedKeyFrame &translationFrame = frames[state.translationFrame];        
        translationFrame.translation = rebaseTranslation(state.translation, restingState.translation);
        translationFrame.translationSet = true;

        DetachedKeyFrame &rotationFrame = frames[state.rotationFrame];
        rotationFrame.rotation = rebase(state.rotation, restingState.rotation);
        rotationFrame.rotationSet = true;
    }
}

static void parseKeyFrames(const Troika::Skeleton *skeleton,
                           const Troika::Animation &animation,
                           const QVector<BoneRestingState> &boneRestingState,
                           QVector<QMap<int,DetachedKeyFrame> > &keyFramesPerBone)
{

    Troika::AnimationStream *stream = animation.openStream(skeleton);

    while (!stream->atEnd()) {
        copyBoneStateToFrames(stream, boneRestingState, keyFramesPerBone);
        if (!stream->readNextFrame())
            break;
    }

    /* Dump the last state into the keyframe at the end of the animation */
    copyBoneStateToFrames(stream, boneRestingState, keyFramesPerBone);

    animation.freeStream(stream);

}

static void fillSparseKeyframes(QMap<int, DetachedKeyFrame> &keyFrames)
{
    QVector3D lastTranslation(0,0,0);
    QVector3D lastScale(1,1,1);
    QQuaternion lastRotation;
    int lastTranslationFrame = -1, lastScaleFrame = -1, lastRotationFrame = -1;

    typedef QMap<int,DetachedKeyFrame>::iterator iterator;

    /* Thankfully, the map is sorted in ascending order */
    iterator it = keyFrames.begin();

    for (; it != keyFrames.end(); ++it) {

        DetachedKeyFrame &frame = it.value();

        // Fill up rotate if last-rotate ever existed
        if (!frame.rotationSet && lastRotationFrame != -1)
        {
            // Search for next quaternion
            iterator itNext = it;
            itNext++;

            for (; itNext != keyFrames.end(); ++itNext)
            {
                DetachedKeyFrame nextFrame = itNext.value();

                if (!nextFrame.rotationSet)
                    continue;

                QQuaternion nextRotation = nextFrame.rotation;

                // -> Found a frame ahead of us that has a rotation set. Now interpolate between us and it
                int frameDiff = itNext.key() - lastRotationFrame; // i.e. frame 1 has rotation, frame 3 has rotation -> diff is 2.
                float factor = (float) (it.key() - lastRotationFrame)/frameDiff;
                frame.rotation = QQuaternion::nlerp(lastRotation, nextRotation, factor);
                frame.rotationSet = true;
                break;
            }

            /*
             * If we were unable to find a frame in the future that has the rotation set, we keep the last rotation constant.
             */
            if (!frame.rotationSet) {
                frame.rotation = lastRotation;
                frame.rotationSet = true;
            }
        }
        else if (frame.rotationSet)
        {
            lastRotationFrame = it.key();
            lastRotation = frame.rotation;
        }

        // Fill up translation if last-scale ever existed
        if (!frame.translationSet && lastTranslationFrame != -1)
        {
            iterator itNext = it;
            itNext++;

            for (; itNext != keyFrames.end(); ++itNext) {
                DetachedKeyFrame nextFrame = itNext.value();

                if (!nextFrame.translationSet)
                    continue;

                QVector3D nextTranslate = nextFrame.translation;

                // -> Found a frame ahead of us that has a trans set. Now interpolate between us and it
                int frameDiff = itNext.key() - lastTranslationFrame; // i.e. frame 1 has trans, frame 3 has trans -> diff is 2.
                float factor = (float)(it.key() - lastTranslationFrame) / frameDiff;
                frame.translation = lastTranslation + (nextTranslate - lastTranslation) * factor;
                frame.translationSet = true;
                break;
            }

            // If no translation ahead could be found, keep the last one constant
            if (!frame.translationSet) {
                frame.translation = lastTranslation;
                frame.translationSet = true;
            }
        }
        else if (frame.translationSet)
        {
            lastTranslationFrame = it.key();
            lastTranslation = frame.translation;
        }

        // TODO: Scale
    }
}
