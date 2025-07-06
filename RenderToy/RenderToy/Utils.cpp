#include "Utils.h"

std::filesystem::path Utils::g_workingDir;

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