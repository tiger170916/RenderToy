#include "Material2.h"

Material2::Material2(
	std::string name,
	std::string baseColorTexName,
	std::string metallicTexName,
	std::string roughnessTexName,
	std::string normalTexName) 
	: m_name(name), m_baseColorTexName(baseColorTexName), m_metallicTexName(metallicTexName), m_roughnessTexName(roughnessTexName), m_normalTexName(normalTexName) {
}

bool Material2::StreamIn(TextureManager2* texManager)
{
	if (m_streamedIn)
	{
		return true;
	}

	if (!texManager)
	{
		return false;
	}

	Texture2* baseColorTex = texManager->GetTexture(m_baseColorTexName);
	Texture2* metallicTex = texManager->GetTexture(m_metallicTexName);
	Texture2* roughnessTex = texManager->GetTexture(m_roughnessTexName);
	Texture2* normalTex = texManager->GetTexture(m_normalTexName);

	if (baseColorTex)
	{
		baseColorTex->AddStreamingReference();
	}

	if (metallicTex)
	{
		metallicTex->AddStreamingReference();
	}

	if (roughnessTex)
	{
		roughnessTex->AddStreamingReference();
	}

	if (normalTex)
	{
		normalTex->AddStreamingReference();
	}

	m_streamedIn = true;
	return true;
}

bool Material2::StreamOut() 
{
	return false;
}