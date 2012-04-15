#ifndef OGRESYSTEMS_H
#define OGRESYSTEMS_H

namespace Ogre {
class LogManager;
class Math;
class LodStrategyManager;
class MaterialManager;
class SkeletonManager;
class MeshSerializer;
class SkeletonSerializer;
class DefaultHardwareBufferManager;
class MeshManager;
class ResourceGroupManager;
class Mesh;
class Skeleton;
}

using Ogre::LogManager;
using Ogre::Math;
using Ogre::LodStrategyManager;
using Ogre::MaterialManager;
using Ogre::MeshSerializer;
using Ogre::SkeletonManager;
using Ogre::SkeletonSerializer;
using Ogre::DefaultHardwareBufferManager;
using Ogre::MeshManager;
using Ogre::ResourceGroupManager;

class OgreSystems
{
public:
    OgreSystems();
    ~OgreSystems();

    // Crappy globals
    // NB some of these are not directly used, but are required to
    //   instantiate the singletons used in the dlls
    LogManager* logMgr;
    Math* mth;
    LodStrategyManager *lodMgr;
    MaterialManager* matMgr;
    SkeletonManager* skelMgr;
    MeshSerializer* meshSerializer;
    SkeletonSerializer* skeletonSerializer;
    DefaultHardwareBufferManager *bufferManager;
    MeshManager* meshMgr;
    ResourceGroupManager* rgm;
};

#endif // OGRESYSTEMS_H
