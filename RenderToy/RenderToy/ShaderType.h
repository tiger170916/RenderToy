#pragma once
#include <string>
enum class ShaderType
{
    SHADER_TYPE_NONE = 0,
    EARLY_Z_PASS_ROOT_SIGNATURE = 1,
    EARLY_Z_PASS_VERTEX_SHADER = 2,
    GEOMETRY_PASS_PIXEL_SHADER = 3,
    GEOMETRY_PASS_ROOT_SIGNATURE = 4,
    GEOMETRY_PASS_VERTEX_SHADER = 5,
    LIGHTING_PASS_PIXEL_SHADER = 6,
    LIGHTING_PASS_ROOT_SIGNATURE = 7,
    LIGHTING_PASS_VERTEX_SHADER = 8,
};
std::wstring GetShaderRelativePath(const ShaderType & type);
