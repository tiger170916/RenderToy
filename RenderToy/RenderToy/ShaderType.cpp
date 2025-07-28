#include "ShaderType.h"
std::wstring GetShaderRelativePath(const ShaderType & type)
{
    switch (type)
    {
    case ShaderType::SHADER_TYPE_NONE:
    {
        return std::wstring();
    }
    case ShaderType::EARLY_Z_PASS_ROOT_SIGNATURE:
    {
        return std::wstring(L"early_z_pass_root_signature.cso");
    }
    case ShaderType::EARLY_Z_PASS_VERTEX_SHADER:
    {
        return std::wstring(L"early_z_pass_vertex_shader.cso");
    }
    case ShaderType::GEOMETRY_PASS_PIXEL_SHADER:
    {
        return std::wstring(L"geometry_pass_pixel_shader.cso");
    }
    case ShaderType::GEOMETRY_PASS_ROOT_SIGNATURE:
    {
        return std::wstring(L"geometry_pass_root_signature.cso");
    }
    case ShaderType::GEOMETRY_PASS_VERTEX_SHADER:
    {
        return std::wstring(L"geometry_pass_vertex_shader.cso");
    }
    case ShaderType::LIGHTING_PASS_PIXEL_SHADER:
    {
        return std::wstring(L"lighting_pass_pixel_shader.cso");
    }
    case ShaderType::LIGHTING_PASS_ROOT_SIGNATURE:
    {
        return std::wstring(L"lighting_pass_root_signature.cso");
    }
    case ShaderType::LIGHTING_PASS_VERTEX_SHADER:
    {
        return std::wstring(L"lighting_pass_vertex_shader.cso");
    }
    case ShaderType::SHADOW_PASS_GEOMETRY_SHADER:
    {
        return std::wstring(L"shadow_pass_geometry_shader.cso");
    }
    case ShaderType::SHADOW_PASS_PIXEL_SHADER:
    {
        return std::wstring(L"shadow_pass_pixel_shader.cso");
    }
    case ShaderType::SHADOW_PASS_ROOT_SIGNATURE:
    {
        return std::wstring(L"shadow_pass_root_signature.cso");
    }
    case ShaderType::SHADOW_PASS_VERTEX_SHADER:
    {
        return std::wstring(L"shadow_pass_vertex_shader.cso");
    }
    }
    return std::wstring();
}
