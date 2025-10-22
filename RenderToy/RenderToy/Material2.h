#pragma once

#include "Includes.h"
#include "TextureManager2.h"

class Material2
{
private:
	std::string m_name;

	std::string m_baseColorTexName;

	std::string m_metallicTexName;

	std::string m_roughnessTexName;

	std::string m_normalTexName;

	bool m_streamedIn = false;

public:
	Material2(
		std::string name,
		std::string baseColorTexName,
		std::string metallicTexName, 
		std::string roughnessTexName, 
		std::string normalTexName);

	inline const std::string& GetName() const { return m_name; }
	inline const std::string& GetBaseColorTexName() const { return m_baseColorTexName; }
	inline const std::string& GetMetallicTexName() const { return m_metallicTexName; }
	inline const std::string& GetRoughnessTexName() const { return m_roughnessTexName; }
	inline const std::string& GetNormalTexName() const { return m_normalTexName; }

	bool StreamIn(TextureManager2* texManager);

	bool StreamOut();
};