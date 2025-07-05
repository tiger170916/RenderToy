#include "ShaderType.h"
std::string GetShaderRelativePath(const ShaderType & type)
{
    switch (type)
    {
    case ShaderType::SHADER_TYPE_NONE:
    {
        return std::string();
    }
    case ShaderType::TEXTSHADERCS:
    {
        return std::string("TextShaderCs.cso");
    }
    }
    return std::string();
}
