#include "ResourceCompilerModuleApi.h"

int main(int argc, char* argv[])
{
	if (argc <= 1 || argv[1] == nullptr)
	{
		return false;
	}

	if (!CompileResource(argv[1]))
	{
		OutputDebugStringA("Failed to compile resource.");
		return 1;
	}

	OutputDebugStringA("resource compiled successfully.");

	return 0;
}