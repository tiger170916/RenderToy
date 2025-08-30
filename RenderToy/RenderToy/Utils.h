#pragma once

#include "Includes.h"
#include "PassType.h"

/// <summary>
/// Utils static class
/// </summary>
class Utils
{
private:
	static std::filesystem::path g_workingDir;

	static std::map<std::string, PassType> g_passTypeStringLookup;

public:
	static std::filesystem::path GetWorkingDirectory();

	static PassType GetPassTypeFromString(std::string str);

	static uint64_t GetCurrentTimeInMicroSec();
};