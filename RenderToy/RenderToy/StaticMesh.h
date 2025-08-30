#pragma once

#include "Includes.h"
#include "Vectors.h"
#include "D3DResource.h"
#include "ConstantBuffer.h"
#include "MeshStructs.h"
#include "PassType.h"
#include "Material.h"
#include "UidGenerator.h"
#include "TextureManager.h"
#include "StreamInterface.h"
#include "ResourceStreamer.h"
#include "Lights/PointLight.h"

class StaticMesh : public StreamInterface
{
friend class MeshFactory;

public:
	struct MeshVertex
	{
		FVector3 Position;
		FVector3 Normal;
		FVector2 UvSets[8];
	};

private:
	struct MeshInstanceStruct
	{
		Transform transform;
		uint32_t uid;
		std::unique_ptr<LightExtension> lightExtension;
	};

private:
	uint32_t m_meshUid = UINT32_MAX;

	static std::map<PassType, std::map<UINT, UINT>> _passMeshArgumentsMap;

	std::map<int, std::vector<StaticMesh::MeshVertex>> m_meshParts; // material id : mesh

	std::vector<std::unique_ptr<Material>> m_materials;

	int m_numUvs = 0;

	// Instances
	std::vector<std::unique_ptr<MeshInstanceStruct>> m_instances;

	std::set<PassType> m_enabledPasses;

	// Vertex buffer resource
	std::vector<std::unique_ptr<D3DResource>> m_vbResources;

	std::vector<std::unique_ptr<D3DResource>>  m_simpleVbResources;

	std::vector<UINT> m_vertexCounts;

	std::vector<D3D12_VERTEX_BUFFER_VIEW> m_vertexBufferViews;

	std::vector<D3D12_VERTEX_BUFFER_VIEW> m_simpleVertexBufferViews;

	std::unique_ptr<ConstantBuffer<MeshInstanceConstants>> m_instanceConstants;

	bool m_selected = false;

	//std::vector<std::shared_ptr<LightExtension>> m_lightExtensions;

	// The mesh marked for destory should not be used anymore.
	bool m_markedForDestroy = false;


public:
	// Stream Interface implementation
	virtual bool StreamIn(GraphicsContext* graphicsContext) override;

	virtual bool StreamOut() override;

	virtual bool ScheduleForCopyToDefaultHeap(ID3D12GraphicsCommandList* cmdList) override;

protected:
	StaticMesh(uint32_t meshUid);

	void AddInstance(const Transform& transform, uint32_t uid);

public:
	~StaticMesh();

	void AddTriangle(const int& part, const MeshVertex& v1, const MeshVertex& v2, const MeshVertex& v3);

	void AddMaterial(Material* material);

	void SetNumUvs(const int num) { m_numUvs = num; }

	void EnablePass(const PassType& renderPass);

	bool PassEnabled(const PassType& renderPass);

	bool BuildResource(GraphicsContext* graphicsContext, TextureManager* textureManager);

	bool UpdateBuffers();

	void Draw(GraphicsContext* graphicsContext, ID3D12GraphicsCommandList* cmdList, PassType passType, bool useSimpleVertex, bool setTextures);

	void AttachLightExtension(LightExtension* light, const uint32_t& instanceIdx);

	bool HasLightExtensions() const;

	const bool HasLightExtension(const uint32_t& instanceIdx);

	LightExtension* GetLightExtension(const uint32_t& instanceIdx);

	void QueueStreamingTasks(ResourceStreamer* streamer, UINT priority);

	inline void MarkForDestory() { m_markedForDestroy = true; }

	inline const bool& IsMarkedForDestroy() const { return m_markedForDestroy; }

	inline void SetSelected(const bool selected) { m_selected = selected; }

	inline const bool& IsSelected() const { return m_selected; }


	// Instance getters
	inline uint32_t GetNumInstances() const { return (uint32_t)m_instances.size(); }

	bool GetInstanceTransform(uint32_t instanceIdx, Transform& outTransform);

	bool GetInstanceUid(uint32_t instanceIdx, uint32_t& outUid);
};