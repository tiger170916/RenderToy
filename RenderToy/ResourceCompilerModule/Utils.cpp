#include "Utils.h"

std::map<std::string, LightType> Utils::_lightTypes = 
{
	{"Spot", LightType::SPOT_LIGHT},
	{"Point", LightType::POINT_LIGHT} ,
	{"Area", LightType::AREA_LIGHT} ,
	{"Directional", LightType::DIRECTIONAL_LIGHT} ,
};

LightType Utils::GetLightTypeFromString(std::string str)
{
	if (_lightTypes.contains(str))
	{
		return _lightTypes[str];
	}

	return LightType::NONE;
}