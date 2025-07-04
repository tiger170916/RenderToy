#include "ShaderType.h"

std::string GetShaderRelativePath(const ShaderType& type)
{
	switch (type)
	{
	case ShaderType::SHADER_TYPE_NONE:
		return std::string("none");
	}

	return std::string();
}