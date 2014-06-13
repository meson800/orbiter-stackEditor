#include "VesselStack.h"

VesselStack::VesselStack(VesselSceneNode* startingVessel)
{
	//recurse through with the helper, start with a null pointer, so any docked port works
	createStackHelper(startingVessel, 0);
}

void VesselStack::changeDockingPortVisibility(bool showEmpty, bool showDocked)
{
	for (unsigned int i = 0; i < nodes.size(); i++)
		nodes[i]->changeDockingPortVisibility(showEmpty, showDocked);
}

int VesselStack::numVessels()
{
	return nodes.size();
}

VesselSceneNode* VesselStack::getVessel(int index)
{
	if (index >= nodes.size())
		return 0;
	return nodes[index];
}

void VesselStack::rotateStack(core::vector3df relativeRot)
{
	//apply this rotation to each of the parent nodes
	core::quaternion x, y, z, finalRot;
	x.fromAngleAxis(relativeRot.X * core::DEGTORAD, core::vector3df(1, 0, 0));
	y.fromAngleAxis(relativeRot.Y * core::DEGTORAD, core::vector3df(0, 1, 0));
	z.fromAngleAxis(relativeRot.Z * core::DEGTORAD, core::vector3df(0, 0, 1));
	finalRot = x * y * z;

	//now use the force, erm, quaternoins to rotate each node in the stack to avoid gimbal lock
	for (unsigned int i = 0; i < nodes.size(); i++)
	{
		core::quaternion thisNodeRotation = core::quaternion(nodes[i]->getRotation() * core::DEGTORAD);
		//rotate this sucker
		thisNodeRotation = thisNodeRotation * finalRot;
		//set the rotation
		core::vector3df eulerRotation;
		thisNodeRotation.toEuler(eulerRotation);
		nodes[i]->setRotation(eulerRotation * core::RADTODEG);
	}

	//now that the nodes are rotated into the correct directions, move them so they line up correctly
	//first, find the center
	core::aabbox3d<f32> overallBox;
	for (unsigned int i = 0; i < nodes.size(); i++)
		overallBox.addInternalBox(nodes[i]->getTransformedBoundingBox());
	core::vector3df center = overallBox.getCenter();
	//now, compare each node's position to the center, rotate the relative position by the overall rotation, and translate it
	for (unsigned int i = 0; i < nodes.size(); i++)
	{
		nodes[i]->updateAbsolutePosition();
		core::vector3df relativePos = nodes[i]->getAbsolutePosition() - center;
		core::vector3df rotatedPos = finalRot * relativePos;
		nodes[i]->setPosition(nodes[i]->getPosition() + (rotatedPos - relativePos));
	}
}

void VesselStack::setMoveReference(core::vector3df refPos)
{
	//set the reference
	moveReference = refPos;
	//set the inital positions of each node
	previousPositions.clear();
	for (unsigned int i = 0; i < nodes.size(); i++)
	{
		previousPositions.push_back(nodes[i]->getPosition());
	}
}

void VesselStack::moveStackRelative(core::vector3df movePos)
{
	//set reference to a null position, and just run the normal referenced move
	//NOTE: this destroys the reference, don't use this in the middle of using moveStackReferenced
	setMoveReference(core::vector3df());
	moveStackReferenced(movePos);
}

void VesselStack::moveStackReferenced(core::vector3df movePos)
{
	//We have to do this system from the reference point because of the way snapping is handled
	//If this just added a relative position to each of the nodes, a small mouse movement would never
	//be able to move the node out of the snapping radius, resulting in permenant snapping.
	//This way always moves the node to a position relative to the initial position, making snapping work

	//check to make sure that we have the same amount of positions in previousPositions than we have nodes
	if (nodes.size() != previousPositions.size())
	{
		Helpers::writeToLog(std::string("Tried to move a vessel stack without setting up previousPositions\n"));
		return;
	}

	//loop over, setting relative position
	for (unsigned int i = 0; i < nodes.size(); i++)
	{
		nodes[i]->setPosition(previousPositions[i] + (movePos - moveReference));
	}

}

void VesselStack::checkForSnapping(std::vector<VesselSceneNode*>& vessels, bool dock)
{
	//loop over each node
	for (unsigned int nodeNum = 0; nodeNum < nodes.size(); nodeNum++)
	{
		//loop over empty docking ports on this node
		for (unsigned int i = 0; i < nodes[nodeNum]->dockingPorts.size(); i++)
		{
			if (nodes[nodeNum]->dockingPorts[i].docked == false)
			{
				//now, loop over the OTHER vessels (not equal to this node)
				//and see if it is close to another port's empty docking ports
				for (unsigned int j = 0; j < vessels.size(); j++)
				{
					//make sure this vessel isn't in our stack
					if (std::find(nodes.begin(), nodes.end(), vessels[j]) == nodes.end())
					{
						//check it's docking ports
						for (unsigned int k = 0; k < vessels[j]->dockingPorts.size(); k++)
						{
							//check if it is not docked, and they are within a certain distance
							if (!vessels[j]->dockingPorts[k].docked &&
								((nodes[nodeNum]->getAbsolutePosition() + nodes[nodeNum]->returnRotatedVector(nodes[nodeNum]->dockingPorts[i].position)) -
								(vessels[j]->getAbsolutePosition() + vessels[j]->returnRotatedVector(vessels[j]->dockingPorts[k].position))
								).getLengthSQ() < 16)
							{
								//snap it if dock=false, dock if dock=true
								if (!dock)
									nodes[nodeNum]->snap(nodes[nodeNum]->dockingPorts[i], vessels[j]->dockingPorts[k]);
								else
									nodes[nodeNum]->dock(nodes[nodeNum]->dockingPorts[i], vessels[j]->dockingPorts[k]);
							}

						}
					}
				}
			}
		}
	}
}

void snap(OrbiterDockingPort& ourPort, OrbiterDockingPort& theirPort)
{
	//First, we need to find the rotation which would allow us to get into position.

	//update both absolute positions
	ourPort.parent->updateAbsolutePosition();
	theirPort.parent->updateAbsolutePosition();

	//this is the first rotation
	core::quaternion initialAlignRotation;

	//get the rotated port direction, times -1 to reverse it so it will match with the other port
	core::vector3df ourPortDirection = -1 * ourPort.parent->returnRotatedVector(ourPort.approachDirection);
	//get the other rotated port vector
	core::vector3df otherPortDirection = theirPort.parent->returnRotatedVector(theirPort.approachDirection);
	//get our first quaternion
	initialAlignRotation.rotationFromTo(ourPortDirection, otherPortDirection);

	//if (!(initialAlignRotation.W == 0 || (initialAlignRotation.X == 0 && initialAlignRotation.Y == 0 && initialAlignRotation.Z == 0)))
	//	setRotation(getRotation() + rotationInEuler);

}

//recursive function to init a vessel stack
void VesselStack::createStackHelper(VesselSceneNode* startingVessel, OrbiterDockingPort* fromPort)
{
	//recurse through the docking ports, through docked vessels
	//if the nodes doesn't contain this vessel yet, add it
	if (std::find(nodes.begin(), nodes.end(), startingVessel) == nodes.end())
	{
		nodes.push_back(startingVessel);
	}

	//base case if there are no occupied docking ports except fromPort
	//recurse if there is an occupied docking port not equal to fromPort

	for (unsigned int i = 0; i < startingVessel->dockingPorts.size(); i++)
	{
		//if we are docked to a port that is not fromPort, recurse into it
		if (startingVessel->dockingPorts[i].docked && startingVessel->dockingPorts[i].dockedTo != fromPort)
			createStackHelper(startingVessel->dockingPorts[i].dockedTo->parent,
				&(startingVessel->dockingPorts[i]));
	}

	//we must be in the base case!
	return;
}