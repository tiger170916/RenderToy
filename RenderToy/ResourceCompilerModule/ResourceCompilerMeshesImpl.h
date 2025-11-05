#pragma once

#include "ResourceCompilerMeshes.h"
#include <vector>
#include <memory>

using namespace ResourceCompilerModule;

class MaterialImpl : public Material
{
private:
	std::string m_materialName;

	std::string m_baseColorTextureName;
	std::string m_baseColorTextureFile;

	std::string m_metallicTextureName;
	std::string m_metallicTextureFile;

	std::string m_roughnessTextureName;
	std::string m_roughnessTextureFile;

	std::string m_normalTextureName;
	std::string m_normalTextureFile;

public:
	inline virtual std::string GetMaterialName() override { return m_materialName; }

	inline virtual std::string GetBaseColorTextureName() override { return m_baseColorTextureName; }
	inline virtual std::string GetBaseColorTextureFile() override { return m_baseColorTextureFile; }

	inline virtual std::string GetMetallicTextureName() override { return m_metallicTextureName; }
	inline virtual std::string GetMetallicTextureFile() override { return m_metallicTextureFile; }

	inline virtual std::string GetRoughnessTextureName() override { return m_roughnessTextureName; }
	inline virtual std::string GetRoughnessTextureFile() override { return m_roughnessTextureFile; }

	inline virtual std::string GetNormalTextureName() override { return m_normalTextureName; }
	inline virtual std::string GetNormalTextureFile() override { return m_normalTextureFile; }

	inline void SetBaseColorTextureName(const std::string& name) { m_baseColorTextureName = name; }
	inline void SetBaseColorTextureFile(const std::string& name) { m_baseColorTextureFile = name; }

	inline void SetMetallicTextureName(const std::string& name) { m_metallicTextureName = name; }
	inline void SetMetallicTextureFile(const std::string& name) { m_metallicTextureFile = name; }

	inline void SetRoughnessTextureName(const std::string& name) { m_roughnessTextureName = name; }
	inline void SetRoughnessTextureFile(const std::string& name) { m_roughnessTextureFile = name; }

	inline void SetNormalTextureName(const std::string& name) { m_normalTextureName = name; }
	inline void SetNormalTextureFile(const std::string& name) { m_normalTextureFile = name; }

	MaterialImpl(std::string name);
};

class MeshPartImpl : public MeshPart
{
private:
	uint32_t m_dataOffset;
	uint32_t m_dataSize;
	uint32_t m_numVertices;
	std::shared_ptr<Material> m_material = nullptr;

public:
	inline virtual uint32_t GetDataOffset() override { return m_dataOffset; }

	inline virtual uint32_t GetDataSize() override { return m_dataSize; }

	inline virtual uint32_t GetNumVertices() override { return m_numVertices; }

	inline virtual Material* GetMaterial() override { return m_material.get(); }

	MeshPartImpl(const uint32_t& dataOffset, const uint32_t& dataSize, const uint32_t& numVertices);

	inline void SetMaterial(std::shared_ptr<Material> material) { m_material = material; }
};

class MeshImpl : public Mesh
{
private:
	std::string m_name;
	std::vector<std::shared_ptr<MeshPart>> m_meshParts;

public:
	inline virtual std::string GetName() override { return m_name; }

	virtual void GetMeshParts(MeshPart** ppMeshParts, uint32_t* pNumParts) override;

	MeshImpl(std::string name);

	inline void AddMeshPart(std::shared_ptr<MeshPart> meshPart) { m_meshParts.push_back(meshPart); }
};

class MeshesDataImpl : public MeshesData
{
private:
	std::vector<std::shared_ptr<Mesh>> m_meshes;
public:
	virtual void GetMeshes(Mesh** ppMeshes, uint32_t* pNumMeshes) override;

	inline void AddMesh(std::shared_ptr<Mesh> mesh) { m_meshes.push_back(mesh); }
};