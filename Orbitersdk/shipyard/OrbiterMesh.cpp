#include "OrbiterMesh.h"

OrbiterMesh::OrbiterMesh()
{}

OrbiterMesh::OrbiterMesh(string meshFilename, video::IVideoDriver* driver)
{
	setupMesh(meshFilename, driver);
}

void OrbiterMesh::setupMesh(string meshFilename, video::IVideoDriver* driver)
{
	ifstream meshFile = ifstream(meshFilename.c_str());
	int groupCounter = 0;
	int materialCounter = 0;
	int textureCounter = 0;
	int meshGroupCounter = 0;
	int vertexCounter = 0;
	int triangleCounter = 0;
	int stage = 0;
	string currentLine;

	//push the default material
	materials.push_back(video::SMaterial());
	//push the default texture
	textures.push_back(driver->addTexture(core::dimension2d<u32>(1, 1),"empty_texture"));

	vector<string> tokens;
	//start reading the meshgroup
	while (Helpers::readLine(meshFile, tokens))
	{
		//split this into stages
		switch (stage)
		{
		case 0:
			//stage zero is defining the meshgroups
			//see if this is the main group definition
			if (strcmp(tokens[0].c_str(), "GROUPS") == 0)
			{
				int numGroups = Helpers::stringToInt(tokens[1]);
				for (int i = 0; i < numGroups; i++)
					meshGroups.push_back(OrbiterMeshGroup());
				break;
			}
			//see if this is our material/texture index
			if (strcmp(tokens[0].c_str(), "MATERIAL") == 0)
			{
				//link the material index
				meshGroups[groupCounter].materialIndex = Helpers::stringToInt(tokens[1]);
				break;
			}
			if (strcmp(tokens[0].c_str(), "TEXTURE") == 0)
			{
				//link the texture index
				meshGroups[groupCounter].textureIndex = Helpers::stringToInt(tokens[1]);
				break;
			}

			//see if this is the geometry one
			if (strcmp(tokens[0].c_str(), "GEOM") == 0)
			{
				//increment the counters
				vertexCounter = Helpers::stringToInt(tokens[1]);
				triangleCounter = Helpers::stringToInt(tokens[2]);
				break;
			}

			//now see if this is a vertex
			if (vertexCounter > 0 && triangleCounter > 0)
			{
				if (tokens.size() == 8)
					meshGroups[groupCounter].vertices.push_back(video::S3DVertex(
					Helpers::stringToDouble(tokens[0]), Helpers::stringToDouble(tokens[1]),
					Helpers::stringToDouble(tokens[2]), Helpers::stringToDouble(tokens[3]),
					Helpers::stringToDouble(tokens[4]), Helpers::stringToDouble(tokens[5]),
					video::SColor(), Helpers::stringToDouble(tokens[6]), Helpers::stringToDouble(tokens[7])));
				//it's a vertex!
				if (tokens.size() == 6)
					meshGroups[groupCounter].vertices.push_back(video::S3DVertex(
					Helpers::stringToDouble(tokens[0]), Helpers::stringToDouble(tokens[1]),
					Helpers::stringToDouble(tokens[2]), Helpers::stringToDouble(tokens[3]),
					Helpers::stringToDouble(tokens[4]), Helpers::stringToDouble(tokens[5]),
					video::SColor(),0,0));
				vertexCounter--;
				break;
			}
			if (vertexCounter == 0 && triangleCounter > 0)
			{
				//it's a triangle!
				meshGroups[groupCounter].triangleList.push_back(Helpers::stringToInt(tokens[0]));
				meshGroups[groupCounter].triangleList.push_back(Helpers::stringToInt(tokens[1]));
				meshGroups[groupCounter].triangleList.push_back(Helpers::stringToInt(tokens[2]));
				triangleCounter--;
				if (triangleCounter == 0)
				{
					//we finished this meshgroup!
					groupCounter++;

					//check if we are done with meshgroups
					if (groupCounter == meshGroups.size() && meshGroups.size() != 0)
					{
						//we're done reading meshgroups!!!
						stage++;
						break;
					}
				}

				break;
			}

			//something went wrong, just break
			break;

		case 1:
			//now we are reading materials
			//see if this is the main Material line with the number of materials
			if (strcmp(tokens[0].c_str(), "MATERIALS") == 0)
			{
				int numMaterials = Helpers::stringToInt(tokens[1]);
				for (int i = 0; i < numMaterials; i++)
					materials.push_back(video::SMaterial());
				break;
			}
			//otherwise see if this is the main material declaration
			if (strcmp(tokens[0].c_str(), "MATERIAL") == 0)
			{
				//read the next four lines
				for (int i = 0; i < 4; i++)
					Helpers::readLine(meshFile, tokens);
				//now the next 17 values should be in tokens[2]-tokens[18]

				//orbiter format
				//2		3	4	5		Diffuse colour(RGBA)
				//6		7	8	9		Ambient colour(RGBA)
				//10	11	12	13	14 Specular colour(RGBA) and specular power(float)
				//15	16	17	18		Emissive colour(RGBA)
				if (tokens.size() == 19)
				{
					materials[materialCounter].DiffuseColor = video::SColor(Helpers::stringToInt(tokens[5]),
						Helpers::stringToInt(tokens[2]), Helpers::stringToInt(tokens[3]), Helpers::stringToInt(tokens[4]));
					materials[materialCounter].AmbientColor = video::SColor(Helpers::stringToInt(tokens[9]),
						Helpers::stringToInt(tokens[6]), Helpers::stringToInt(tokens[7]), Helpers::stringToInt(tokens[8]));
					materials[materialCounter].SpecularColor = video::SColor(Helpers::stringToInt(tokens[13]),
						Helpers::stringToInt(tokens[10]), Helpers::stringToInt(tokens[11]), Helpers::stringToInt(tokens[12]));
					//set specular power-"shineness"
					materials[materialCounter].Shininess = Helpers::stringToDouble(tokens[14]);
					materials[materialCounter].EmissiveColor = video::SColor(Helpers::stringToInt(tokens[18]),
						Helpers::stringToInt(tokens[15]), Helpers::stringToInt(tokens[16]), Helpers::stringToInt(tokens[17]));
				}
				if (tokens.size() == 18)
				{
					materials[materialCounter].DiffuseColor = video::SColor(Helpers::stringToInt(tokens[5]),
						Helpers::stringToInt(tokens[2]), Helpers::stringToInt(tokens[3]), Helpers::stringToInt(tokens[4]));
					materials[materialCounter].AmbientColor = video::SColor(Helpers::stringToInt(tokens[9]),
						Helpers::stringToInt(tokens[6]), Helpers::stringToInt(tokens[7]), Helpers::stringToInt(tokens[8]));
					materials[materialCounter].SpecularColor = video::SColor(Helpers::stringToInt(tokens[13]),
						Helpers::stringToInt(tokens[10]), Helpers::stringToInt(tokens[11]), Helpers::stringToInt(tokens[12]));
					//set specular power-"shineness"
					materials[materialCounter].Shininess = 20;
					materials[materialCounter].EmissiveColor = video::SColor(Helpers::stringToInt(tokens[17]),
						Helpers::stringToInt(tokens[14]), Helpers::stringToInt(tokens[15]), Helpers::stringToInt(tokens[16]));
				}

				//we're done!
				materialCounter++;
				if (materialCounter == materials.size() && materials.size() != 0)
					stage++;
				break;
			}

		case 2:
			//now we are loading textures
			if (strcmp(tokens[0].c_str(), "TEXTURES") == 0)
			{
				textureCounter = Helpers::stringToInt(tokens[1]);
				break;
			}

			//see if this is a texture path
			if (textureCounter > 0)
			{
				textures.push_back(driver->getTexture(string("C:\\Other Stuff\\Orbiter\\shipyard\\Textures\\" + tokens[0]).c_str()));
				textureCounter--;
				break;
			}
			break;

		}

		//clean up the token vector
		tokens.clear();
	}

	//now, set up the bounding box
	boundingBox.reset(meshGroups[0].vertices[0].Pos);
	//loop over every vertex
	for (int i = 0; i < meshGroups.size(); i++)
	{
		for (int j = 0; j < meshGroups[i].vertices.size(); j++)
			boundingBox.addInternalPoint(meshGroups[i].vertices[j].Pos);
	}
}