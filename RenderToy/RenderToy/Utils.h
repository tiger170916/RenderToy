#pragma once

#include "Includes.h"

/// <summary>
/// Utils static class
/// </summary>
class Utils
{
private:
	static std::filesystem::path g_workingDir;

public:
	static std::filesystem::path GetWorkingDirectory();
};