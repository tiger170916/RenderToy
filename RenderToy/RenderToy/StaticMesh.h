#pragma once

#include "Includes.h"
#include "Vectors.h"
#include "RenderPassType.h"
#include "D3DResource.h"
#include "ConstantBuffer.h"
#include "MeshStructs.h"

class StaticMesh
{
private:
	std::vector<FVector3> m_points;

	std::vector<int> m_triangles;

	std::vector<Transform> m_instances;

	std::set<RenderPass> m_enabledPasses;

	// TODO: change to multiple resources
	// Vertex buffer resource
	std::unique_ptr<D3DResource> m_vbResource = nullptr;

	// Index buffer resource
	std::unique_ptr<D3DResource> m_ibResource = nullptr;

	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView = {};

	D3D12_INDEX_BUFFER_VIEW m_indexBufferView = {};

	std::unique_ptr<ConstantBuffer<MeshInstanceConstants>> m_instanceConstants = nullptr;

	std::vector<XMMATRIX> m_transformMatrices;

public:
	StaticMesh();

	~StaticMesh();

	void AddPoint(float& x, float& y, float& z);

	void AddTriangle(int& v1, int& v2, int& v3);

	void AddInstance(const Transform& transform);

	void EnableRenderPass(const RenderPass& renderPass);

	bool PassEnabled(const RenderPass& renderPass);

	bool BuildResource(ID3D12Device* pDevice, DescriptorHeapManager* descriptorHeapManager);

	void Draw(ID3D12GraphicsCommandList* cmdList, DescriptorHeapManager* descriptorHeapManager);

};