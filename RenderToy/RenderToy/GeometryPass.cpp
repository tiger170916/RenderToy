#include "GeometryPass.h"
#include "GraphicsUtils.h"
#include "Macros.h"

GeometryPass::GeometryPass()
{

}

GeometryPass::~GeometryPass()
{

}

bool GeometryPass::Initialize(GraphicsContext* graphicsContext, ShaderManager* shaderManager, PipelineResourceStates* pipelineResourceStates)
{
	if (m_initialized)
	{
		return true;
	}

	if (!graphicsContext || !shaderManager || !pipelineResourceStates)
	{
		return false;
	}

	const UINT& width = graphicsContext->GetHwndWidth();
	const UINT& height = graphicsContext->GetHwndHeight();

	ID3D12Device* pDevice = graphicsContext->GetDevice();
	UINT adapterNodeMask = graphicsContext->GetAdapterNodeMask();
	DescriptorHeapManager* descHeapManager = graphicsContext->GetDescriptorHeapManager();

	if (!GraphicsUtils::CreateDepthStencilResource(
		pDevice, 
		descHeapManager, 
		width,
		height, 
		m_depthStencilFormat, 
		D3D12_RESOURCE_FLAG_NONE, 
		1.0f, 
		0,
		m_depthStencilBuffer.GetAddressOf(), 
		m_dsvId))
	{
		return false;
	}

	pipelineResourceStates->AddResourceState(m_depthStencilBuffer.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE);

	if (!GraphicsUtils::CreateRenderTargetResource(
		pDevice,
		descHeapManager,
		width,
		height,
		m_diffuseRenderTargetFormat,
		D3D12_RESOURCE_FLAG_NONE,
		m_diffuseBufferClearValue,
		m_diffuseBuffer.GetAddressOf(),
		m_diffuseRtvId))
	{
		return false;
	}

	pipelineResourceStates->AddResourceState(m_diffuseBuffer.Get(), D3D12_RESOURCE_STATE_PRESENT);

	m_graphicsPipelineState = std::unique_ptr<GraphicsPipelineState>(new GraphicsPipelineState());
	D3D12_GRAPHICS_PIPELINE_STATE_DESC& pipelineStateDesc = m_graphicsPipelineState->GraphicsPipelineStateDesc();

	pipelineStateDesc.NodeMask = adapterNodeMask;
	pipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	pipelineStateDesc.DSVFormat = m_depthStencilFormat;
	pipelineStateDesc.NumRenderTargets = 1;
	pipelineStateDesc.RTVFormats[0] = m_diffuseRenderTargetFormat;

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

	if (!m_graphicsPipelineState->Initialize(graphicsContext, shaderManager, ShaderType::GEOMETRY_PASS_ROOT_SIGNATURE, ShaderType::GEOMETRY_PASS_VERTEX_SHADER, ShaderType::GEOMETRY_PASS_PIXEL_SHADER))
	{
		return false;
	}

	m_viewport = D3D12_VIEWPORT{ 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
	m_scissorRect = { 0, 0, (long)width, (long)height };

	m_initialized = true;

	return true;
}

void GeometryPass::Frame(World* world, ID3D12GraphicsCommandList* commandList, GraphicsContext* graphicsContext, PipelineResourceStates* pipelineResourceStates, PipelineOutputsStruct& outputs)
{
	if (world == nullptr || commandList == nullptr || graphicsContext == nullptr)
	{
		return;
	}

	DescriptorHeapManager* descHeapManager = graphicsContext->GetDescriptorHeapManager();
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;
	descHeapManager->GetDepthStencilViewCpuHandle(m_dsvId, dsvHandle);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvs[4];
	descHeapManager->GetRenderTargetViewCpuHandle(m_diffuseRtvId, rtvs[0]);

	if (!GraphicsUtils::PipelineResourceBarrierTransition(m_diffuseBuffer.Get(), pipelineResourceStates, commandList, D3D12_RESOURCE_STATE_RENDER_TARGET))
	{
		return;
	}

	// Set pso
	commandList->SetPipelineState(m_graphicsPipelineState->GetPipelineState());
	commandList->SetGraphicsRootSignature(m_graphicsPipelineState->GetRootSignature());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	commandList->ClearRenderTargetView(rtvs[0], m_diffuseBufferClearValue, 0, NULL);
	commandList->OMSetRenderTargets(1, rtvs, false, &dsvHandle);
	commandList->RSSetViewports(1, &m_viewport);
	commandList->RSSetScissorRects(1, &m_scissorRect);

	// Bind uniform frame constant buffer
	D3D12_GPU_DESCRIPTOR_HANDLE uniformFrameGpuHandle;

	world->GetUniformFrameConstantBuffer()->BindConstantBufferViewToPipeline(graphicsContext, uniformFrameGpuHandle);
	commandList->SetGraphicsRootDescriptorTable(0, uniformFrameGpuHandle);

	if (outputs.Outputs.contains(PipelinePassOutputType::EARLY_Z_PASS_DEPTH_BUFFER))
	{
		PipelineOutput& earlyZPassDepthBufferOutput = outputs.Outputs[PipelinePassOutputType::EARLY_Z_PASS_DEPTH_BUFFER];

		// Change the state of earlyZBuffer to COMMON 
		GraphicsUtils::PipelineResourceBarrierTransition(earlyZPassDepthBufferOutput.pResource, pipelineResourceStates, commandList, D3D12_RESOURCE_STATE_COMMON);

		D3D12_GPU_DESCRIPTOR_HANDLE earlyZPassBufferHandle;
		descHeapManager->BindCbvSrvUavToPipeline(earlyZPassDepthBufferOutput.SrvId, earlyZPassBufferHandle);
		commandList->SetGraphicsRootDescriptorTable(2, earlyZPassBufferHandle);
	}

	for (auto& staticMesh : world->GetAllStaticMeshes())
	{
		if (!staticMesh->PassEnabled(RenderPass::GEOMETRY_PASS))
		{
			continue;
		}

		staticMesh->Draw(graphicsContext, commandList);
	}

	// Output
}