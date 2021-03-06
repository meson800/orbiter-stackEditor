//Copyright (c) 2015 Christopher Johnstone(meson800) and Benedict Haefeli(jedidia)
//The MIT License - See ../../LICENSE for more info
#include "Helpers.h"

std::map<unsigned int, VesselSceneNode*>* Helpers::vesselMap = 0;

std::string Helpers::workingDirectory = "";
StackEditor* Helpers::mainStackEditor = 0;
IrrlichtDevice *Helpers::irrdevice = NULL;
std::mutex Helpers::videoDriverMutex;
bool Helpers::readLine(ifstream& file, std::vector<std::string>& tokens, const std::string &delimiters)
{
	std::string line;

	//return if we reached eof
	if (!getline(file, line))
		return false;
	//cut everything beyond a ';'
	if (line.find_first_of(';') != std::string::npos)
		line.erase(line.find_first_of(';'), std::string::npos);
	//remove extra spaces
	removeExtraSpaces(line);
	tokenize(line, tokens, delimiters);
	return true;
}

void Helpers::tokenize(std::string line, std::vector<std::string>& tokens, const std::string &delimiters)
{
	// Skipping delimiters at the beginning
	std::string::size_type lastPos = line.find_first_not_of(delimiters, 0);
	// Find first "non-delimiter".
	std::string::size_type pos = line.find_first_of(delimiters, lastPos);

	while (pos != std::string::npos || lastPos != std::string::npos)
	{
		// Found a token, add it to the vector.
		tokens.push_back(line.substr(lastPos, pos - lastPos));
		// Skip delimiters.  Note the "not_of"
		lastPos = line.find_first_not_of(delimiters, pos);
		// Find next "non-delimiter"
		pos = line.find_first_of(delimiters, lastPos);
	}
}

int Helpers::stringToInt(const std::string& inputString)
{
	int num;
	std::istringstream(inputString) >> num;
	return num;
}
double Helpers::stringToDouble(const std::string& inputString)
{
	double num;
	std::istringstream(inputString) >> num;
	return num;
}

video::ITexture* Helpers::readDDS(std::string path, std::string name, video::IVideoDriver* driver)
{

	videoDriverMutex.lock();
	//get the DDS
	irrutils::DdsImage ddsImage = irrutils::DdsImage(path.c_str(), driver);
	video::IImage* image = ddsImage.getImage();
	video::ITexture* texture = driver->addTexture(name.c_str(), image);
	videoDriverMutex.unlock();
	texture->grab();
	return texture;
}

void Helpers::removeExtraSpaces(std::string& str)
{
	std::string::iterator new_end = std::unique(str.begin(), str.end(), BothAreSpaces);
	str.erase(new_end, str.end());
}

//takes the name of the mesh as noted in the config file and returns the name of the GUI image for that mesh
//basically, replaces all "/" or "\\" with _ and appends ".bmp"
std::string Helpers::meshNameToImageName(std::string meshname)
{
	std::string imagename = "";
	std::string::size_type pos = meshname.find_first_of("/\\", 0);
	std::string::size_type lastpos = meshname.find_first_not_of("/\\", 0);
	while (pos != std::string::npos)
	{
		imagename = imagename + meshname.substr(lastpos, pos - lastpos) + "_";
		lastpos = meshname.find_first_not_of("/\\", pos);
		pos = meshname.find_first_of("/\\", lastpos);
	}
	imagename = imagename + meshname.substr(lastpos, meshname.length()) + ".bmp";
	return imagename;
}

//loads windowresolution and used toolboxset from config file
CONFIGPARAMS Helpers::loadConfigParams()
{
	CONFIGPARAMS params;
	params.windowres = core::dimension2d<u32>(0, 0);
	params.toolboxset = "default";
	std::string cfgPath("./StackEditor/StackEditor.cfg");
	ifstream configFile = ifstream(cfgPath.c_str());

    Log::clearLog();
    Log::writeToLog("Initialising StackEditor...");
    Log::writeToLog("Build Date: ",Version::build_date);
    Log::writeToLog("Build Version: ",Version::build_git_version);
	if (configFile)
	{
		std::vector<std::string> tokens;
		while (readLine(configFile, tokens))
		{
			if (tokens.size() == 0) continue;

			if (tokens[0].compare("resolution") == 0 && tokens.size() == 3)
			{
				params.windowres = core::dimension2d<u32>(unsigned int(Helpers::stringToInt(tokens[1])), unsigned int(Helpers::stringToInt(tokens[2])));
			}

			if (tokens[0].compare("tbxset") == 0)
			{
				if (tokens.size() > 2)
				//putting together the rest of the string again if the folder name contains a space
				{
					params.toolboxset = "";
					for (unsigned int i = 1; i < tokens.size(); ++i)
					{
						params.toolboxset += std::string(tokens[i] + " ");
					}
				}
				else
				{
					params.toolboxset = tokens[1];
				}
			}

            if (tokens[0].compare("loglevel") == 0)
            {
                if (tokens.size() < 2)
                {
                    Log::writeToLog(Log::WARN, "Missing loglevel setting, defaulting to WARN");
                }
                else if (tokens.size() >= 2)
                {
                    std::string level = tokens[1];
                    std::transform(level.begin(), level.end(), level.begin(), ::tolower);
                    std::map<std::string, Log::LogLevel> stringToLevel = { { "all", Log::ALL }, { "debug", Log::L_DEBUG }, { "info", Log::INFO },
                    { "warn", Log::WARN }, { "warning", Log::WARN }, { "error", Log::ERR }, { "fatal", Log::FATAL }, { "off", Log::OFF } };

                    if (stringToLevel.count(level))
                    {
                        Log::setLogLevel(stringToLevel[level]);
                    }
                    else
                    {
                        Log::writeToLog(Log::WARN, "Bad loglevel setting, defaulting to WARN");
                    }

                    if (tokens.size() > 2)
                    {
                        Log::writeToLog(Log::WARN, "Unusual loglevel setting, too many arguments");
                    }
                }
            }

			tokens.clear();
		}
		configFile.close();
	}
	else
	{
        Log::writeToLog(Log::WARN, "StackEditor.cfg not found!");
	}
	return params;
}


//fun fact: irrlicht changes the working directory when using a fileopendialog
//we totally need to reset that, or Orbiter won't find ANY files anymore.
void Helpers::resetDirectory()
{
	irrdevice->getFileSystem()->changeWorkingDirectoryTo(Helpers::workingDirectory.c_str());
}

//replaces all / in a path with \\. Does not work for leading and trailing slashes.
void Helpers::slashreplace(std::string &str)
{
	std::vector<std::string> tokens;
	tokenize(str, tokens, "/");
	str = "";
	for (unsigned int i = 0; i < tokens.size() - 1; ++i)
	{
		str = str + tokens[i] + "\\";
	}
	str += tokens[tokens.size() - 1];
}

void Helpers::setVesselMap(std::map<unsigned int, VesselSceneNode*>* _vesselMap)
{
    vesselMap = _vesselMap;
}

void Helpers::registerVessel(unsigned int uid, VesselSceneNode* vessel)
{
    if (vesselMap != 0)
    {
        (*vesselMap)[uid] = vessel;
    }
}

void Helpers::unregisterVessel(unsigned int uid)
{
    if (vesselMap != 0)
    {
        vesselMap->erase(uid);
    }
}

VesselSceneNode* Helpers::getVesselByUID(unsigned int uid)
{
    if (vesselMap != 0)
    {
        return vesselMap->at(uid);
    }
    return 0;
}

bool Helpers::isUIDRegistered(unsigned int uid)
{
    if (vesselMap != 0)
    {
        return vesselMap->count(uid) != 0;
    }
    return false;
}

unsigned int Helpers::findFreeUID(unsigned int startNum)
{
    while (isUIDRegistered(startNum))
        ++startNum;
    return startNum;
}
