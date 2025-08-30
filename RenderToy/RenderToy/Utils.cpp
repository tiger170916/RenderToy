#include "Utils.h"

std::filesystem::path Utils::g_workingDir;

std::map<std::string, PassType> Utils::g_passTypeStringLookup
{
	{"SHADOW_PASS", PassType::SHADOW_PASS },
	{"EARLY_Z_PASS", PassType::EARLY_Z_PASS },
	{"GEOMETRY_PASS", PassType::GEOMETRY_PASS },
	{"LIGHTING_PASS", PassType::LIGHTING_PASS },
	{"LIGHT_SHAFT_PRE_PASS", PassType::LIGHT_SHAFT_PRE_PASS },
	{"LIGHT_FRUSTUM_DEBUG_PASS", PassType::LIGHT_FRUSTUM_DEBUG_PASS },
	{"LIGHT_SHAFT_PASS", PassType::LIGHT_SHAFT_PASS },
	{"SILHOUETTE_RENDER_PASS", PassType::SILHOUETTE_RENDER_PASS },
};

std::filesystem::path Utils::GetWorkingDirectory()
{
	if (g_workingDir.empty())
	{
		HMODULE hModule = GetModuleHandle(nullptr);
		if (!hModule) {
			return g_workingDir;
		}

		wchar_t path[MAX_PATH];
		GetModuleFileNameW(hModule, path, sizeof(path));
		g_workingDir = path;
		g_workingDir = g_workingDir.parent_path();
	}

	return g_workingDir;
}

PassType Utils::GetPassTypeFromString(std::string str)
{
	if (g_passTypeStringLookup.contains(str))
	{
		return g_passTypeStringLookup[str];
	}

	return PassType::NONE;
}

uint64_t Utils::GetCurrentTimeInMicroSec()
{
	// Get the count in microseconds
	auto now = std::chrono::high_resolution_clock::now();
	auto duration_since_epoch = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch());
	return duration_since_epoch.count();
}