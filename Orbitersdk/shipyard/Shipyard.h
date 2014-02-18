#pragma once

#include <irrlicht.h>

#include "VesselSceneNode.h"

#define cameraRotateSpeed = .1;

using namespace irr;

class Shipyard : public IEventReceiver
{
public:
	void setupDevice(IrrlichtDevice * _device);
	void loop();
	bool OnEvent(const SEvent & event);

private:
	core::vector3df returnMouseRelativePos();
	
	IrrlichtDevice * device;
	bool LMouseDown;
	scene::ICameraSceneNode* camera;
	scene::ISceneNode * selectedNode;
	scene::ISceneCollisionManager* collisionManager;
	scene::ISceneManager* smgr;
	core::vector3df oldMouse3DPos;
};