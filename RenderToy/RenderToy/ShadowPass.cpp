#include "ShadowPass.h"

ShadowPass::ShadowPass(GUID passGuid)
	: RenderPassBase(passGuid)
{
	m_numTiles = m_depthAtlasBufferSize / m_l1ShadowMapSize;
}

ShadowPass::~ShadowPass() {}

// Interface
bool ShadowPass::Initialize(GraphicsContext* graphicsContext, ShaderManager* shaderManager)
{
	if (m_initialized)
	{
		return true;
	}

	if (!graphicsContext || !shaderManager)
	{
		return false;
	}

	if (!PassBase::Initialize(graphicsContext, shaderManager))
	{
		return false;
	}

	ID3D12Device* pDevice = graphicsContext->GetDevice();
	DescriptorHeapManager* descHeapManager = graphicsContext->GetDescriptorHeapManager();

	// Create the depth/stencil buffer and view.
	D3D12_RESOURCE_DESC depthAtlasResourceDesc = {};
	depthAtlasResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthAtlasResourceDesc.Alignment = 0;
	depthAtlasResourceDesc.Width = m_depthAtlasBufferSize;
	depthAtlasResourceDesc.Height = m_depthAtlasBufferSize;
	depthAtlasResourceDesc.DepthOrArraySize = 1;
	depthAtlasResourceDesc.MipLevels = 1;
	depthAtlasResourceDesc.SampleDesc.Count = 1;
	depthAtlasResourceDesc.SampleDesc.Quality = 0;
	depthAtlasResourceDesc.Format = m_depthFormat;
	depthAtlasResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthAtlasResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	if (FAILED(pDevice->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
		&depthAtlasResourceDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(m_depthAtalasBuffer.GetAddressOf()))))
	{
		return false;
	}

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = m_depthFormat;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	uavDesc.Texture2D.PlaneSlice = 0;

	m_depthAtlasUavId = descHeapManager->CreateUnorderedAccessView(m_depthAtalasBuffer.Get(), nullptr, &uavDesc);

	return true;
}

bool ShadowPass::PopulateCommands(World* world, GraphicsContext* graphicsContext)
{
	m_skiped = true;

	if (world == nullptr || graphicsContext == nullptr)
	{
		return false;
	}

	PassBase::PopulateCommands(world, graphicsContext);

	ID3D12GraphicsCommandList* commandList = m_commandBuilder->GetCommandList();

	m_commandBuilder->Close();

	return true;
}