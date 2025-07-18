#pragma once

#include "Includes.h"
#include "Vectors.h"
#include "D3DResource.h"
#include "ConstantBuffer.h"
#include "MeshStructs.h"
#include "PassType.h"

class StaticMesh
{
private:
	std::vector<FVector3> m_points;

	std::vector<int> m_triangles;

	// Currently instance only contains transform data (extent to a struct)
	std::vector<Transform> m_instances;

	std::set<PassType> m_enabledPasses;

	// TODO: change to multiple resources
	// Vertex buffer resource
	std::unique_ptr<D3DResource> m_vbResource = nullptr;

	// Index buffer resource
	std::unique_ptr<D3DResource> m_ibResource = nullptr;

	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView = {};

	D3D12_INDEX_BUFFER_VIEW m_indexBufferView = {};

	std::unique_ptr<ConstantBuffer<MeshInstanceConstants>> m_instanceConstants = nullptr;

public:
	StaticMesh();

	~StaticMesh();

	void AddPoint(const float& x, const float& y, const float& z);

	void AddTriangle(const int& v1, const int& v2, const int& v3);

	void AddInstance(const Transform& transform);

	void EnablePass(const PassType& renderPass);

	bool PassEnabled(const PassType& renderPass);

	bool BuildResource(GraphicsContext* graphicsContext);

	void Draw(GraphicsContext* graphicsContext, ID3D12GraphicsCommandList* cmdList);

};