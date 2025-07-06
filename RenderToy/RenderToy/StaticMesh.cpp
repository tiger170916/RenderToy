#include "StaticMesh.h"

StaticMesh::StaticMesh()
{

}

StaticMesh::~StaticMesh()
{

}

void StaticMesh::AddPoint(float& x, float& y, float& z)
{
	m_points.push_back(FVector3(x, y, z));
}

void StaticMesh::AddTriangle(int& v1, int& v2, int& v3)
{
	m_triangles.push_back(v1);
	m_triangles.push_back(v2);
	m_triangles.push_back(v3);
}

void StaticMesh::AddInstance(const Transform& transform)
{
	m_instances.push_back(transform);
}

void StaticMesh::EnableRenderPass(const RenderPass& renderPass)
{
	m_enabledPasses.insert(renderPass);
}

bool StaticMesh::PassEnabled(const RenderPass& renderPass)
{
	return m_enabledPasses.contains(renderPass);
}

bool StaticMesh::BuildResource(ID3D12Device* pDevice)
{
	// Build vertex buffer resources
	m_vbResource = std::unique_ptr<D3DResource>(new D3DResource(true));

	std::vector<MeshVertex> vertices;
	for (int i = 0; i < m_points.size(); i++)
	{
		MeshVertex v = {};
		v.position[0] = m_points[i].X;
		v.position[1] = m_points[i].Y;
		v.position[2] = m_points[i].Z;

		vertices.push_back(v);
	}

	auto vbDesc = CD3DX12_RESOURCE_DESC::Buffer(vertices.size() * sizeof(MeshVertex));
	if (!m_vbResource->Initialize(pDevice, &vbDesc, vertices.data(), (UINT)vertices.size() * sizeof(MeshVertex)))
	{
		return false;
	}

	// Build index buffer resources
	m_ibResource = std::unique_ptr<D3DResource>(new D3DResource(true));
	auto ibDesc = CD3DX12_RESOURCE_DESC::Buffer(m_triangles.size() * sizeof(int));
	if (!m_ibResource->Initialize(pDevice, &ibDesc, m_triangles.data(), (UINT)m_triangles.size() * sizeof(int)))
	{
		return false;
	}

	m_vertexBufferView.BufferLocation = m_vbResource->GetDefaultResource()->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = sizeof(MeshVertex);
	m_vertexBufferView.SizeInBytes = (UINT)vertices.size() * sizeof(MeshVertex);

	m_indexBufferView.BufferLocation = m_ibResource->GetDefaultResource()->GetGPUVirtualAddress();
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT; // 32-bit unsigned integer
	m_indexBufferView.SizeInBytes = (UINT)m_triangles.size() * sizeof(UINT);

	m_instanceConstants = std::unique_ptr<ConstantBuffer<MeshInstanceConstants>>(new ConstantBuffer<MeshInstanceConstants>(64));
	m_instanceConstants->Initialize(pDevice);

	return true;
}

void StaticMesh::DrawCall(ID3D12GraphicsCommandList* cmdList)
{
	if (!cmdList)
	{
		return;
	}

	const D3D12_VERTEX_BUFFER_VIEW* pVbViews = &m_vertexBufferView;
	const D3D12_INDEX_BUFFER_VIEW* pIbView = &m_indexBufferView;
	cmdList->IASetVertexBuffers(0, 1, pVbViews);
	cmdList->IASetIndexBuffer(pIbView);
	cmdList->DrawIndexedInstanced((UINT)m_triangles.size(), (UINT)m_instances.size(), 0, 0, 0);
}