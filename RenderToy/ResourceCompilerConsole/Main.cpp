#include "ResourceCompilerModuleApi.h"

int main(int argc, char* argv[])
{
	if (argc <= 1)
	{
		return false;
	}

	for (int i = 1; i < argc; i++)
	{
		if (argv[i] == nullptr)
		{
			continue;
		}

		if (!CompileResource(argv[i]))
		{
			OutputDebugStringA("Failed to compile resource.");
		}
	}

	OutputDebugStringA("resource compiled successfully.");

	return 0;
}