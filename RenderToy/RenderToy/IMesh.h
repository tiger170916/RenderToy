#pragma once

#include "Includes.h"
#include "IDrawable.h"
#include "SceneObjectComponent.h"
#include "ConstantBuffer.h"
#include "MeshStructs.h"

/// <summary>
/// Mesh Interface
/// </summary>
class IMesh : public SceneObjectComponent, public IDrawable
{
private:
	// Whether the mesh is from file.
	bool m_isFromeFile = false;
	// The file path (if from file)
	std::filesystem::path m_filePath;
	std::vector<uint32_t> m_meshPartDataOffsets;
	std::vector<uint32_t> m_meshPartDataSizes;
	std::vector<std::string> m_MeshPartMaterialNames;

protected:
	// Internal struct for static mesh part
	struct MeshPartStruct
	{
		uint32_t vertexCount = 0;

		uint32_t bufferSize = 0;

		std::unique_ptr<D3DResource> pResource;

		D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};

		std::string materialName;
	};

	static std::map<PassType, std::map<UINT, UINT>> _passMeshArgumentsMap;

	std::string m_meshName;

	std::vector<std::unique_ptr<MeshPartStruct>> m_meshParts;

	std::unique_ptr<ConstantBuffer<MeshInstanceConstants>> m_instanceConstants;

	bool m_isUploadedToGpu = false;

	uint32_t m_uid = UINT_MAX;

public:
	IMesh(std::string meshName);

	~IMesh();

	void SetIsFromFile(std::filesystem::path filePath);

	void AddFileMetadataPart(UINT offset, UINT size, std::string materialName);

	inline const std::string& GetName() const { return m_meshName; }

	inline const bool IsUploadedToGpu() const { return m_isUploadedToGpu; }

	inline const uint32_t GetUid() const { return m_uid; }

	inline const std::filesystem::path GetFilePath() { return m_filePath; }

	/// <summary>
	/// Load the mesh from file
	/// </summary>
	bool LoadFromBinary(
		GraphicsContext* graphicsContext,
		MaterialManager* materialManager,
		TextureManager2* texLoader,
		CommandBuilder* cmdBuilder);

	// Interfaces
	virtual bool UpdateBuffersForFrame() = 0;
};