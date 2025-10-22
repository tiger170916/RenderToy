#pragma once

#include "Includes.h"
#include "Material2.h"

/// <summary>
/// Material manager class
/// </summary>
class MaterialManager
{
private:
	std::map<std::string, std::unique_ptr<Material2>> m_materials;

public:
	MaterialManager() {};
	~MaterialManager() {};

	void CreateMaterial(std::string name, std::string baseColorTexName, std::string metallicTexName, std::string roughnessTexName, std::string normalTexName);

	Material2* GetMaterial(std::string name);
};