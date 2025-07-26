#include "Material.h"

Material::Material()
{
	
}

bool Material::Initialize(GraphicsContext* graphicsContext, TextureManager* textureManager)
{
	m_materialConstantBuffer = std::unique_ptr<ConstantBuffer<MaterialConstants>>(new ConstantBuffer<MaterialConstants>());
	if (!m_materialConstantBuffer->Initialize(graphicsContext))
	{
		return false;
	}

	if (std::filesystem::exists(m_baseColorTexturePath))
	{
		m_baseColorTexture = textureManager->LoadTexture(m_baseColorTexturePath);
	}

	if (std::filesystem::exists(m_metallicTexturePath))
	{
		m_metallicTexture = textureManager->LoadTexture(m_metallicTexturePath);
	}

	if (std::filesystem::exists(m_roughnessTexturePath))
	{
		m_roughnessTexture = textureManager->LoadTexture(m_roughnessTexturePath);
	}

	if (std::filesystem::exists(m_normalTexturePath))
	{
		m_normalTexture = textureManager->LoadTexture(m_normalTexturePath);
	}

	//if (std::filesystem::exists(m_ambientOcclusionTexturePath))
	//{
	//	m_ambientOcclusionTexture = textureManager->LoadTexture(m_ambientOcclusionTexturePath);
	//}

	m_initialized = true;

	return true;
}

Material::~Material()
{

}