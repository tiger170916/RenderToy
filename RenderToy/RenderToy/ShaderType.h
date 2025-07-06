#pragma once
#include <string>
enum class ShaderType
{
    SHADER_TYPE_NONE = 0,
    EARLY_Z_PASS_ROOT_SIGNATURE = 1,
};
std::string GetShaderRelativePath(const ShaderType & type);
