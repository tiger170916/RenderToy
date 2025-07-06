#pragma once

#include "Includes.h"
#include "ShaderType.h"

class ShaderManager
{
private:
	class ShaderStruct
	{
	public:
		char* m_data = nullptr;
		UINT m_dataSize = 0;

	public:
		ShaderStruct(char* data, UINT dataSize);
		~ShaderStruct();
	};

private:
	std::map<ShaderType, std::unique_ptr<ShaderStruct>> m_cachedShaders;

public:
	bool GetShader(const ShaderType& shaderType, char** ppData, UINT& dataSize);

};