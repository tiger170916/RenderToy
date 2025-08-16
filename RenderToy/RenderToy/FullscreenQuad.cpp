#include "FullscreenQuad.h"
#include "MeshStructs.h"

std::unique_ptr<FullScreenQuad> FullScreenQuad::_singleton = nullptr;


FullScreenQuad* FullScreenQuad::Get()
{
	if (!_singleton)
	{
		_singleton = std::unique_ptr<FullScreenQuad>(new FullScreenQuad());
	}
	return _singleton.get();
}

bool FullScreenQuad::Initialize(GraphicsContext* graphicsContext)
{
	std::vector<FullScreenVertexDx> vertices =
	{
		{{-1.0f, -1.0f}},
		{{ 1.0f,  1.0f}},
		{{ 1.0f, -1.0f}},
		{{-1.0f, -1.0f}},
		{{-1.0f,  1.0f}},
		{{ 1.0f,  1.0f}}
	};

	// Create vb 
	auto vbDesc = CD3DX12_RESOURCE_DESC::Buffer(vertices.size() * sizeof(FullScreenVertexDx));

	m_vbResource = std::unique_ptr<D3DResource>(new D3DResource(true));
	if (!m_vbResource->Initialize(graphicsContext, &vbDesc, vertices.data(), (UINT)vertices.size() * sizeof(FullScreenVertexDx), 0))
	{
		return false;
	}

	// Init vertex buffer view
	m_vertexBufferView.BufferLocation = m_vbResource->GetDefaultResource()->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = sizeof(FullScreenVertexDx);
	m_vertexBufferView.SizeInBytes = (UINT)vertices.size() * sizeof(FullScreenVertexDx);

	m_initialized = true;
	return true;
}

bool FullScreenQuad::Draw(ID3D12GraphicsCommandList* commandList)
{
	if (!m_initialized)
	{
		return false;
	}

	if (!commandList)
	{
		return false;
	}

	m_vbResource->CopyToDefaultHeap(commandList);
	commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	commandList->DrawInstanced(m_vertexCount, 1, 0, 0);

	return true;
}

FullScreenQuad::~FullScreenQuad() {}