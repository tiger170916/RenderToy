#pragma once
#include <string>
enum class ShaderType
{
    SHADER_TYPE_NONE = 0,
    TEXTSHADERCS = 1,
};
std::string GetShaderRelativePath(const ShaderType & type);
