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
#include "DataManager.h"

using namespace irr;
using namespace std;

class VesselSceneNode : public scene::ISceneNode
{
public:
	VesselSceneNode(VesselData *vesData, scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id, 
        UINT _uid = next_uid++); //If UID is not given, init to the next free UID, then increment next_UID

	virtual void OnRegisterSceneNode();
	virtual void render();
	virtual void drawDockingPortLines(video::IVideoDriver* driver);
	virtual const core::aabbox3d<f32>& getBoundingBox() const;
	virtual u32 getMaterialCount();
	virtual video::SMaterial& getMaterial(u32 i);
	void setupDockingPortNodes();
	void changeDockingPortVisibility(bool showEmpty, bool showDocked, bool showHelper = false);
	void snap(OrbiterDockingPort& ourPort, OrbiterDockingPort& theirPort);
	void dock(OrbiterDockingPort& ourPort, OrbiterDockingPort& theirPort);
	core::vector3df returnRotatedVector(const core::vector3df& vec);
	VesselData* returnVesselData();
	void setTransparency(bool transparency);

	OrbiterDockingPort* dockingPortSceneNodeToOrbiter(scene::ISceneNode* sceneNode);
	OrbiterDockingPort* dockingPortHelperNodeToOrbiter(scene::ISceneNode* sceneNode);
	void saveToSession(ofstream &file);
	bool loadFromSession(ifstream &file);
	std::string getClassName();

    UINT getUID();

	vector<OrbiterDockingPort> dockingPorts;

private:
    UINT uid;
    static UINT next_uid;
	scene::ISceneManager* smgr;
	OrbiterMesh *vesselMesh;
	VesselData *vesselData;
	void setupDockingPortNode(IMeshSceneNode *node);
	bool transparent;
};