#pragma once
#include "ResourceStructs.h"
#include <string>
#include <map>

using namespace ResourceCompilerModule;
class Utils
{
private:
	static std::map<std::string, LightType> _lightTypes;
public:
	static LightType GetLightTypeFromString(std::string str);
};