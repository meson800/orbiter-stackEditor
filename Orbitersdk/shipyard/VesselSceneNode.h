#pragma once

#include <irrlicht.h>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

#include "OrbiterMesh.h"
#include "Helpers.h"
#include "OrbiterDockingPort.h"
#include "resource.h"

using namespace irr;
using namespace std;

class VesselSceneNode : public scene::ISceneNode
{
public:
	VesselSceneNode(string configFilename, scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id);
	virtual void OnRegisterSceneNode();
	virtual void render();
	virtual const core::aabbox3d<f32>& getBoundingBox() const;
	virtual u32 getMaterialCount();
	virtual video::SMaterial& getMaterial(u32 i);
	void setupDockingPortNodes();
	void changeDockingPortVisibility(bool show, bool emptyOnly);
	void snap(OrbiterDockingPort& ourPort, OrbiterDockingPort& theirPort);
	core::vector3df returnRotatedVector(const core::vector3df& vec);

private:
	
	vector<OrbiterDockingPort> dockingPorts;
	scene::ISceneManager* smgr;
	OrbiterMesh vesselMesh;
};