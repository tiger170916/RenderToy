#include "ShaderManager.h"
#include "Utils.h"

ShaderManager::ShaderManager() {}

ShaderManager::~ShaderManager() {}

bool ShaderManager::GetShader(const ShaderType& shaderType, char** ppData, UINT& dataSize)
{
	dataSize = 0;

	if (!ppData)
	{
		return false;
	}

	// Get shader from cache
	if (m_cachedShaders.contains(shaderType))
	{
		ShaderStruct* shaderStruct = m_cachedShaders[shaderType].get();
		*ppData = shaderStruct->m_data;
		dataSize = shaderStruct->m_dataSize;

		return true;
	}

	std::wstring shaderName = GetShaderRelativePath(shaderType);
	if (shaderName.empty())
	{
		return false;
	}

	*ppData = nullptr;

	std::filesystem::path workingDir = Utils::GetWorkingDirectory();

	std::filesystem::path hlslPath = workingDir.append(L"Shaders").append(shaderName);

	std::ifstream file(hlslPath, std::ios::in | std::ios::binary | std::ios::ate);

	if (!file.is_open())
	{
		return false;
	}

	UINT size = (UINT)file.tellg();
	char* data = (char*)malloc(sizeof(char) * size);
	memset(data, 0, size);
	
	file.seekg(0, std::ios::beg);
	file.read(data, size);
	file.close();

	m_cachedShaders[shaderType] = std::unique_ptr<ShaderManager::ShaderStruct>(new ShaderManager::ShaderStruct(data, size));

	*ppData = data;
	dataSize = size;

	return true;
}

ShaderManager::ShaderStruct::ShaderStruct(char* data, UINT dataSize)
	: m_data(data), m_dataSize(dataSize)
{}

ShaderManager::ShaderStruct::~ShaderStruct()
{
	free(m_data);
	m_dataSize = 0;
}