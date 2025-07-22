#pragma once

#include "Includes.h"
#include "ConstantBuffer.h"
#include "GraphicsContext.h"
#include "TextureManager.h"

struct MaterialConstants
{
	float BaseColor[4];

	float Metallic;

	float Roughness;
};

class Material
{
private:
	std::filesystem::path m_baseColorTexturePath;
	std::filesystem::path m_metallicTexturePath;
	std::filesystem::path m_roughnessTexturePath;
	std::filesystem::path m_normalTexturePath;
	std::filesystem::path m_ambientOcclusionTexturePath;

	Texture* m_baseColorTexture = nullptr;
	Texture* m_metallicTexture = nullptr;
	Texture* m_roughnessTexture = nullptr;
	Texture* m_normalTexture = nullptr;
	Texture* m_ambientOcclusionTexture = nullptr;

	std::unique_ptr<ConstantBuffer<MaterialConstants>> m_materialConstantBuffer = nullptr;

	bool m_initialized = false;

public:
	Material();
	
	bool Initialize(GraphicsContext* graphicsContext);

	~Material();

	inline ConstantBuffer<MaterialConstants>* GetConstantBuffer() { return m_materialConstantBuffer.get(); };


};