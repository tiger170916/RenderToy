#include "ShaderTypeCodeGen.h"
#include <windows.h>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>

bool ShaderTypeCodeGen::GenerateCode()
{
    std::cout << "Generating ShaderType" << std::endl;

    char buffer[MAX_PATH]; 
    DWORD length = GetModuleFileNameA(NULL, buffer, MAX_PATH);

	std::filesystem::path shaderDir = buffer;
    shaderDir = shaderDir.remove_filename();
	shaderDir = shaderDir.append("Shaders");

    if (!std::filesystem::exists(shaderDir))
    {
        std::cout << "Shaders directory doesn't exist" << std::endl;
        return false;
    }
     
    std::filesystem::path shaderEnumHeaderFilePath = buffer;
    shaderEnumHeaderFilePath = shaderEnumHeaderFilePath.parent_path();
    shaderEnumHeaderFilePath = shaderEnumHeaderFilePath.parent_path();
    shaderEnumHeaderFilePath = shaderEnumHeaderFilePath.append("RenderToy\\ShaderType.h");

    std::vector<std::string> shaderFiles;
    std::vector<std::string> shaderEnums;

    // Create a recursive_directory_iterator for the specified path
    for (const auto& entry : std::filesystem::recursive_directory_iterator(shaderDir)) {
        // Check if the current entry is a regular file
        
        if (std::filesystem::is_regular_file(entry.status())) {

            std::filesystem::path shaderPath = entry.path();

            if (entry.path().extension().string() == ".cso")
            {
                std::string fileName = shaderPath.filename().string();
                std::string fileNameWithoutExt = shaderPath.filename().replace_extension("").string();
                std::transform(fileNameWithoutExt.begin(), fileNameWithoutExt.end(), fileNameWithoutExt.begin(),
                    [](unsigned char c) { return std::toupper(c); });
                shaderFiles.push_back(fileName);
                shaderEnums.push_back(fileNameWithoutExt);

                std::cout << "Found shader type" << fileNameWithoutExt << std::endl;
            }
        }
    }

    std::cout << "Generating ShaderType.h file..." << std::endl;
    std::ofstream headerFile(shaderEnumHeaderFilePath.string().c_str());
    if (!headerFile.is_open()) {
        std::cout<< "Error opening file!" << std::endl;
        return false;
    }

    headerFile.clear();

    headerFile << "#pragma once\n"
               << "#include <string>\n"
               << "enum class ShaderType\n"
               << "{\n"
               << "    SHADER_TYPE_NONE = 0,\n";

    for (int i = 0; i < shaderFiles.size(); i++)
    {
        headerFile << "    " << shaderEnums[i] << " = " << i + 1 << ",\n";
    }

    headerFile << "};\n";
    headerFile << "std::string GetShaderRelativePath(const ShaderType & type);\n";

    headerFile.close();

    return true;
}