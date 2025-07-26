#pragma once

#include "Includes.h"
#include "Vectors.h"
#include "D3DResource.h"
#include "ConstantBuffer.h"
#include "MeshStructs.h"
#include "PassType.h"
#include "Material.h"
#include "TextureManager.h"
#include "StreamInterface.h"
#include "ResourceStreamer.h"
#include "Lights/PointLight.h"

class StaticMesh : public StreamInterface
{
public:
	struct MeshVertex
	{
		FVector3 Position;
		FVector3 Normal;
		FVector2 UvSets[8];
	};

private:
	std::map<int, std::vector<StaticMesh::MeshVertex>> m_meshParts; // material id : mesh

	std::vector<std::unique_ptr<Material>> m_materials;

	int m_numUvs = 0;

	// Currently instance only contains transform data (extent to a struct)
	std::vector<Transform> m_instances;

	std::set<PassType> m_enabledPasses;

	// Vertex buffer resource
	std::vector<std::unique_ptr<D3DResource>> m_vbResources;

	std::vector<UINT> m_vertexCounts;

	std::vector<D3D12_VERTEX_BUFFER_VIEW> m_vertexBufferViews;

	std::unique_ptr<ConstantBuffer<MeshInstanceConstants>> m_instanceConstants;

	std::vector<std::shared_ptr<LightExtension>> m_lightExtensions;

public:
	// Stream Interface implementation
	virtual bool StreamIn(GraphicsContext* graphicsContext) override;

	virtual bool StreamOut() override;

	virtual bool ScheduleForCopyToDefaultHeap(ID3D12GraphicsCommandList* cmdList) override;

public:
	StaticMesh();

	~StaticMesh();

	void AddTriangle(const int& part, const MeshVertex& v1, const MeshVertex& v2, const MeshVertex& v3);

	void AddMaterial(Material* material);

	void SetNumUvs(const int num) { m_numUvs = num; }

	void AddInstance(const Transform& transform);

	void EnablePass(const PassType& renderPass);

	bool PassEnabled(const PassType& renderPass);

	bool BuildResource(GraphicsContext* graphicsContext, TextureManager* textureManager);

	void Draw(GraphicsContext* graphicsContext, ID3D12GraphicsCommandList* cmdList, bool setTextures);

	void AttachLightExtension(LightExtension* light);

	bool HasLightExtensions() const { return !m_lightExtensions.empty(); }

	void QueueStreamingTasks(ResourceStreamer* streamer, UINT priority);

	std::vector<std::shared_ptr<LightExtension>> GetLightExtensions() { return m_lightExtensions; }
};