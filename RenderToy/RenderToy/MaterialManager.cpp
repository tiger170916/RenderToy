#include "MaterialManager.h"

void MaterialManager::CreateMaterial(std::string name, std::string baseColorTexName, std::string metallicTexName, std::string roughnessTexName, std::string normalTexName)
{
	if (GetMaterial(name))
	{
		return;
	}

	m_materials[name] = std::make_unique<Material2>(name, baseColorTexName, metallicTexName, roughnessTexName, normalTexName);
}

Material2* MaterialManager::GetMaterial(std::string name)
{
	if (m_materials.contains(name))
	{
		return m_materials[name].get();
	}

	return nullptr;
}