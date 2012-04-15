
#include <Ogre.h>
#include <OgreMeshSerializer.h>
#include <OgreSkeletonSerializer.h>
#include <OgreDefaultHardwareBufferManager.h>
#include <OgreProgressiveMesh.h>

#include "ogresystems.h"

OgreSystems::OgreSystems() : logMgr(0), rgm(0), mth(0), lodMgr(0), meshMgr(0), matMgr(0),
    skelMgr(0), meshSerializer(0), skeletonSerializer(0),
    bufferManager(0)
{
    logMgr = new LogManager();
    rgm = new ResourceGroupManager();
    mth = new Math();
    lodMgr = new LodStrategyManager();
    meshMgr = new MeshManager();
    matMgr = new MaterialManager();
    matMgr->initialise();
    skelMgr = new SkeletonManager();
    meshSerializer = new MeshSerializer();
    skeletonSerializer = new SkeletonSerializer();
    bufferManager = new DefaultHardwareBufferManager(); // needed because we don't have a rendersystem

    logMgr->createLog("default", true, false, true);
}

OgreSystems::~OgreSystems()
{
    delete skeletonSerializer;
    delete meshSerializer;
    delete skelMgr;
    delete matMgr;
    delete meshMgr;
    delete bufferManager;
    delete lodMgr;
    delete mth;
    delete rgm;
    delete logMgr;
}
