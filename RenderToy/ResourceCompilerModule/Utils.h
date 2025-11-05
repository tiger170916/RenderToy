#pragma once
#include "ResourceHeaders.h"
#include <string>
#include <map>

class Utils
{
private:
	static std::map<std::string, LightType> _lightTypes;
public:
	static LightType GetLightTypeFromString(std::string str);
};