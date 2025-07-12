#include "EarlyZPass.h"
#include "Macros.h"

EarlyZPass::EarlyZPass()
{
}

bool EarlyZPass::Initialize(GraphicsContext* graphicsContext, ShaderManager* shaderMgr, UINT width, UINT height)
{
	if (m_initialized)
	{
		return true;
	}

	if (!graphicsContext || !shaderMgr)
	{
		return false;
	}

	ID3D12Device* pDevice = graphicsContext->GetDevice();
	UINT adapterNodeMask = graphicsContext->GetAdapterNodeMask();
	DescriptorHeapManager* descHeapManager = graphicsContext->GetDescriptorHeapManager();

	m_depthStencilFormat = DXGI_FORMAT_D32_FLOAT;

	// Create the depth/stencil buffer and view.
	D3D12_RESOURCE_DESC depthStencilResourceDesc = {};
	depthStencilResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilResourceDesc.Alignment = 0;
	depthStencilResourceDesc.Width = width;
	depthStencilResourceDesc.Height = height;
	depthStencilResourceDesc.DepthOrArraySize = 1;
	depthStencilResourceDesc.MipLevels = 0;
	depthStencilResourceDesc.SampleDesc.Count = 1;
	depthStencilResourceDesc.SampleDesc.Quality = 0;
	depthStencilResourceDesc.Format = m_depthStencilFormat;
	depthStencilResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear = {};
	optClear.Format = m_depthStencilFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	if (FAILED(pDevice->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&depthStencilResourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&optClear,
		IID_PPV_ARGS(m_depthStencilBuffer.GetAddressOf()))))
	{
		return false;
	}

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = m_depthStencilFormat;
	dsvDesc.Texture2D.MipSlice = 0;

	m_dsvId = descHeapManager->CreateDepthStencilView(m_depthStencilBuffer.Get(), &dsvDesc);
	if (m_dsvId == UINT64_MAX)
	{
		return false;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;
	descHeapManager->GetDepthStencilViewCpuHandle(m_dsvId, dsvHandle);

	pDevice->CreateDepthStencilView(
		m_depthStencilBuffer.Get(),
		&dsvDesc,
		dsvHandle);

	m_graphicsPipelineState = std::unique_ptr<GraphicsPipelineState>(new GraphicsPipelineState());
	D3D12_GRAPHICS_PIPELINE_STATE_DESC& pipelineStateDesc = m_graphicsPipelineState->GraphicsPipelineStateDesc();

	pipelineStateDesc.NodeMask = adapterNodeMask;
	pipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {
		TRUE,
		D3D12_DEPTH_WRITE_MASK_ALL,
		D3D12_COMPARISON_FUNC_LESS_EQUAL,
		FALSE
	};
		
	D3D12_RASTERIZER_DESC rasterizerDesc = {
		D3D12_FILL_MODE_SOLID,
		D3D12_CULL_MODE_NONE,
		FALSE,
		0,
		0.0f, //clamp
		0.0f, //slope scaled
		TRUE,
		FALSE,
		FALSE,
		0,
		D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
	};

	D3D12_INPUT_ELEMENT_DESC meshInputLayout[] = IA_MESH_LAYOUT;

	pipelineStateDesc.RasterizerState = rasterizerDesc;
	pipelineStateDesc.DepthStencilState = depthStencilDesc;
	pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	pipelineStateDesc.SampleDesc.Count = 1;
	pipelineStateDesc.SampleDesc.Quality = 0;
	pipelineStateDesc.SampleMask = UINT_MAX;
	pipelineStateDesc.InputLayout.NumElements = 1;
	pipelineStateDesc.InputLayout.pInputElementDescs = meshInputLayout;

	if (!m_graphicsPipelineState->Initialize(pDevice, adapterNodeMask, shaderMgr, ShaderType::EARLY_Z_PASS_ROOT_SIGNATURE, ShaderType::EARLY_Z_PASS_VERTEX_SHADER, ShaderType::SHADER_TYPE_NONE))
	{
		return false;
	}

	m_initialized = true;

	return true;
}

void EarlyZPass::Frame(std::shared_ptr<World> world, ID3D12GraphicsCommandList* commandList, GraphicsContext* graphicsContext)
{
	if (world == nullptr || commandList == nullptr || graphicsContext == nullptr)
	{
		return;
	}

	DescriptorHeapManager* descHeapManager = graphicsContext->GetDescriptorHeapManager();
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;
	descHeapManager->GetDepthStencilViewCpuHandle(m_dsvId, dsvHandle);

	// Set pso
	commandList->SetPipelineState(m_graphicsPipelineState->GetPipelineState());
	commandList->SetGraphicsRootSignature(m_graphicsPipelineState->GetRootSignature());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	commandList->OMSetRenderTargets(0, nullptr, false, &dsvHandle);

	// Bind uniform frame constant buffer
	D3D12_GPU_DESCRIPTOR_HANDLE uniformFrameGpuHandle;

	world->GetUniformFrameConstantBuffer()->BindConstantBufferViewToPipeline(graphicsContext->GetDescriptorHeapManager(), uniformFrameGpuHandle);
	commandList->SetGraphicsRootDescriptorTable(0, uniformFrameGpuHandle);

	for (auto& staticMesh : world->GetAllStaticMeshes())
	{
		if (!staticMesh->PassEnabled(RenderPass::EARLY_Z_PASS))
		{
			continue;
		}

		staticMesh->Draw(commandList);
	}
}

EarlyZPass::~EarlyZPass()
{
	if (m_depthStencilBuffer)
	{
		m_depthStencilBuffer.Reset();
	}
}