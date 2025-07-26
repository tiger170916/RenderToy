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

	std::shared_ptr<Texture> m_baseColorTexture = nullptr;
	std::shared_ptr<Texture> m_metallicTexture = nullptr;
	std::shared_ptr<Texture> m_roughnessTexture = nullptr;
	std::shared_ptr<Texture> m_normalTexture = nullptr;
	std::shared_ptr<Texture> m_ambientOcclusionTexture = nullptr;

	std::unique_ptr<ConstantBuffer<MaterialConstants>> m_materialConstantBuffer = nullptr;

	bool m_initialized = false;

public:
	Material();

	void SetBaseColorTexturePath(std::filesystem::path path) { m_baseColorTexturePath = path; }

	void SetMetallicTexturePath(std::filesystem::path path) { m_metallicTexturePath = path; }

	void SetRoughnessTexturePath(std::filesystem::path path) { m_roughnessTexturePath = path; }

	void SetNormalTexturePath(std::filesystem::path path) { m_normalTexturePath = path; }

	void SetAmbientOcclusionTexturePath(std::filesystem::path path) { m_ambientOcclusionTexturePath = path; }
	
	bool Initialize(GraphicsContext* graphicsContext, TextureManager* textureManager);

	~Material();

	inline ConstantBuffer<MaterialConstants>* GetConstantBuffer() { return m_materialConstantBuffer.get(); };

	inline std::shared_ptr<Texture> GetBaseColorTexture() { return m_baseColorTexture; }

	inline std::shared_ptr<Texture> GetMetallicTexture() { return m_metallicTexture; }

	inline std::shared_ptr<Texture> GetRoughnessTexture() { return m_roughnessTexture; }

	inline std::shared_ptr<Texture> GetNormalTexture() { return m_normalTexture; }

	inline std::shared_ptr<Texture> GetAmbientOcclusionTexture() { return m_ambientOcclusionTexture; }
};