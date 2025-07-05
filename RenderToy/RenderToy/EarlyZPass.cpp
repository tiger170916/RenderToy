#include "EarlyZPass.h"

EarlyZPass::EarlyZPass()
{
}

bool EarlyZPass::Initialize(ID3D12Device* pDevice, UINT width, UINT height)
{
	if (m_initialized)
	{
		return true;
	}

	m_depthStencilFormat = DXGI_FORMAT_D32_FLOAT;

	// depth stencil view heap.
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	if (FAILED(pDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(m_depthStencilViewHeap.GetAddressOf()))))
	{
		return false;
	}

	// Create the depth/stencil buffer and view.
	D3D12_RESOURCE_DESC depthStencilDesc = {};
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = width;
	depthStencilDesc.Height = height;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = m_depthStencilFormat;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear = {};
	optClear.Format = m_depthStencilFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	if (FAILED(pDevice->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&optClear,
		IID_PPV_ARGS(m_depthStencilBuffer.GetAddressOf()))))
	{
		return false;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_depthStencilViewHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = m_depthStencilFormat;
	dsvDesc.Texture2D.MipSlice = 0;

	pDevice->CreateDepthStencilView(
		m_depthStencilBuffer.Get(),
		&dsvDesc,
		dsvHandle);

	m_initialized = true;

	return true;
}

void EarlyZPass::Frame(std::shared_ptr<World> world)
{
	if (world == nullptr)
	{
		return;
	}

	for (auto& staticMesh : world->GetAllStaticMeshes())
	{
		if (!staticMesh->PassEnabled(RenderPass::EARLY_Z_PASS))
		{
			continue;
		}
	}
}

EarlyZPass::~EarlyZPass()
{
	if (m_depthStencilViewHeap)
	{
		m_depthStencilViewHeap->Release();
		m_depthStencilViewHeap = nullptr;
	}

	if (m_depthStencilBuffer)
	{
		m_depthStencilBuffer->Release();
		m_depthStencilBuffer = nullptr;
	}
}