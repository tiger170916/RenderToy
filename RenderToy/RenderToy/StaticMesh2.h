#pragma once

#include "Includes.h"
#include "GraphicsContext.h"
#include "D3DResource.h"
#include "CommandBuilder.h"
#include "TextureManager2.h"
#include "MaterialManager.h"
#include "ConstantBuffer.h"
#include "MeshStructs.h"
#include "Vectors.h"
#include "PassType.h"
#include "lights/LightExtension.h"
#include <fstream>

class StaticMesh2
{
private:
	// Internal struct for static mesh part
	struct MeshPartStruct
	{
		uint32_t vertexCount;

		uint32_t bufferSize;

		std::unique_ptr<D3DResource> pResource;

		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

		std::string materialName;
	};

	// Internal struct for static mesh instance
	struct InstanceStruct
	{
		Transform transform;
		std::unique_ptr<LightExtension> lightExtension = nullptr;
	};

private:
	static std::map<PassType, std::map<UINT, UINT>> _passMeshArgumentsMap;

	std::string m_meshName;

	std::vector<std::unique_ptr<MeshPartStruct>> m_meshParts;

	std::unique_ptr<ConstantBuffer<MeshInstanceConstants>> m_instanceConstants;

	std::vector<std::unique_ptr<InstanceStruct>> m_instances;

	bool m_isUploadedToGpu = false;

public:
	StaticMesh2(std::string meshName);

	void AddInstance(Transform transform);

	~StaticMesh2();

	const std::string& GetName() const { return m_meshName; }

	const bool IsUploadedToGpu() const { return m_isUploadedToGpu; }

	/// <summary>
	/// Load the mesh from file
	/// </summary>
	bool LoadFromBinary(
		GraphicsContext* graphicsContext,
		MaterialManager* materialManager,
		TextureManager2* texLoader,
		CommandBuilder* cmdBuilder,
		std::fstream* file,
		std::vector<uint32_t> meshPartOffsets,
		std::vector<uint32_t> meshPartDataSizes,
		std::vector<std::string> materialNames);

	bool UpdateBuffersForFrame();

	/// <summary>
	/// Stream out from gpu
	/// </summary>
	bool StreamOut();

	void Draw(GraphicsContext* graphicsContext, MaterialManager* materialManager, TextureManager2* textureManager, ID3D12GraphicsCommandList* cmdList, PassType passType, bool useSimpleVertex, bool setTextures);

	bool AttachLightExtension(uint32_t instanceIdx, std::unique_ptr<LightExtension>& lightExt);

	inline uint32_t GetNumInstances() const { return (uint32_t)m_instances.size(); }

	bool GetInstanceTransform(const uint32_t& idx, Transform& transform) const;

	LightExtension* GetLightExtension(const uint32_t& idx);
};