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
    }
    return std::wstring();
}
