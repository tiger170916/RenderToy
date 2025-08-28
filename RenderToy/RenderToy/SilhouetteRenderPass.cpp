#include "SilhouetteRenderPass.h"
#include "LightingPass.h"
#include "GraphicsUtils.h"
#include "Macros.h"

SilhouetteRenderPass::SilhouetteRenderPass(GUID passGuid)
	: RenderPassBase(passGuid)
{
	m_passType = PassType::SILHOUETTE_RENDER_PASS;
}

SilhouetteRenderPass::~SilhouetteRenderPass()
{

}

bool SilhouetteRenderPass::Initialize(GraphicsContext* graphicsContext, ShaderManager* shaderManager)
{
	if (m_initialized)
	{
		return true;
	}

	if (!graphicsContext || !shaderManager)
	{
		return false;
	}

	LightingPass* dependencyLightingPass = (LightingPass*)GetDependencyPassOfType(PassType::LIGHTING_PASS);
	if (!dependencyLightingPass)
	{
		return false;
	}

	if (!PassBase::Initialize(graphicsContext, shaderManager))
	{
		return false;
	}

	const UINT& width = graphicsContext->GetHwndWidth();
	const UINT& height = graphicsContext->GetHwndHeight();

	ID3D12Device* pDevice = graphicsContext->GetDevice();
	UINT adapterNodeMask = graphicsContext->GetAdapterNodeMask();
	DescriptorHeapManager* descHeapManager = graphicsContext->GetDescriptorHeapManager();

	// Create the depth/stencil buffer and view.
	if (!GraphicsUtils::CreateDepthStencilResource(
		pDevice,
		descHeapManager,
		width,
		height,
		m_depthFormat,
		D3D12_RESOURCE_FLAG_NONE,
		1.0f,
		0,
		m_depthStencilBuffer.GetAddressOf(),
		m_dsvId))
	{
		return false;
	}

	m_resourceStates[m_depthStencilBuffer.Get()] = D3D12_RESOURCE_STATE_DEPTH_WRITE;

	D3D12_INPUT_ELEMENT_DESC meshInputLayout[] = IA_MESH_LAYOUT;

	m_depthPipelineState = std::unique_ptr<GraphicsPipelineState>(new GraphicsPipelineState());
	D3D12_GRAPHICS_PIPELINE_STATE_DESC& depthPipelineStateDesc = m_depthPipelineState->GraphicsPipelineStateDesc();
	depthPipelineStateDesc.NodeMask = adapterNodeMask;
	depthPipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	depthPipelineStateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	depthPipelineStateDesc.DepthStencilState.DepthEnable = true;
	depthPipelineStateDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthPipelineStateDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	depthPipelineStateDesc.DepthStencilState.StencilEnable = false;

	depthPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	depthPipelineStateDesc.DSVFormat = m_depthFormat;
	depthPipelineStateDesc.SampleDesc.Count = 1;
	depthPipelineStateDesc.SampleDesc.Quality = 0;
	depthPipelineStateDesc.SampleMask = UINT_MAX;
	depthPipelineStateDesc.InputLayout.NumElements = NUM_IA_MESH_LAYOUT;
	depthPipelineStateDesc.InputLayout.pInputElementDescs = meshInputLayout;
	depthPipelineStateDesc.NumRenderTargets = 0;

	if (!m_depthPipelineState->Initialize(graphicsContext, shaderManager, ShaderType::SILHOUETTE_RENDER_PASS_ROOT_SIGNATURE, ShaderType::SILHOUETTE_RENDER_PASS_DEPTH_STENCIL_VERTEX_SHADER, ShaderType::SHADER_TYPE_NONE, ShaderType::SHADER_TYPE_NONE))
	{
		return false;
	}

	m_stencilPipelineState = std::unique_ptr<GraphicsPipelineState>(new GraphicsPipelineState());
	D3D12_GRAPHICS_PIPELINE_STATE_DESC& stencilPipelineStateDesc = m_stencilPipelineState->GraphicsPipelineStateDesc();
	stencilPipelineStateDesc = depthPipelineStateDesc;
	stencilPipelineStateDesc.DepthStencilState.DepthEnable = true;
	stencilPipelineStateDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	stencilPipelineStateDesc.DepthStencilState.StencilEnable = true;
	stencilPipelineStateDesc.DepthStencilState.StencilReadMask = 0xFF;
	stencilPipelineStateDesc.DepthStencilState.StencilWriteMask = 0xFF;
	stencilPipelineStateDesc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	stencilPipelineStateDesc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	stencilPipelineStateDesc.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	stencilPipelineStateDesc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
	stencilPipelineStateDesc.DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	stencilPipelineStateDesc.DepthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	stencilPipelineStateDesc.DepthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	stencilPipelineStateDesc.DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
	if (!m_stencilPipelineState->Initialize(graphicsContext, shaderManager, ShaderType::SILHOUETTE_RENDER_PASS_ROOT_SIGNATURE, ShaderType::SILHOUETTE_RENDER_PASS_DEPTH_STENCIL_VERTEX_SHADER, ShaderType::SHADER_TYPE_NONE, ShaderType::SHADER_TYPE_NONE))
	{
		return false;
	}

	m_silhouettePipelineState = std::unique_ptr<GraphicsPipelineState>(new GraphicsPipelineState());
	D3D12_GRAPHICS_PIPELINE_STATE_DESC& silhouettePipelineStateDesc = m_silhouettePipelineState->GraphicsPipelineStateDesc();
	silhouettePipelineStateDesc = stencilPipelineStateDesc;
	silhouettePipelineStateDesc.DepthStencilState.DepthEnable = false;
	silhouettePipelineStateDesc.DepthStencilState.StencilEnable = true;
	silhouettePipelineStateDesc.DepthStencilState.StencilReadMask = 0xFF;
	silhouettePipelineStateDesc.DepthStencilState.StencilWriteMask = 0x00;
	silhouettePipelineStateDesc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_GREATER;
	silhouettePipelineStateDesc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	silhouettePipelineStateDesc.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	silhouettePipelineStateDesc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	silhouettePipelineStateDesc.DepthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_GREATER;
	silhouettePipelineStateDesc.DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	silhouettePipelineStateDesc.DepthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	silhouettePipelineStateDesc.DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	silhouettePipelineStateDesc.NumRenderTargets = 1;
	silhouettePipelineStateDesc.RTVFormats[0] = dependencyLightingPass->GetRenderTargetFormat();
	silhouettePipelineStateDesc.BlendState.RenderTarget[0].BlendEnable = true;
	silhouettePipelineStateDesc.BlendState.RenderTarget[0].LogicOpEnable = false;
	silhouettePipelineStateDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	silhouettePipelineStateDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	silhouettePipelineStateDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	silhouettePipelineStateDesc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	silhouettePipelineStateDesc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
	silhouettePipelineStateDesc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
	silhouettePipelineStateDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	if (!m_silhouettePipelineState->Initialize(graphicsContext, shaderManager, ShaderType::SILHOUETTE_RENDER_PASS_ROOT_SIGNATURE, ShaderType::SILHOUETTE_RENDER_PASS_VERTEX_SHADER, ShaderType::SILHOUETTE_RENDER_PASS_GEOMETRY_SHADER, ShaderType::SILHOUETTE_RENDER_PASS_PIXEL_SHADER))
	{
		return false;
	}

	m_viewport = D3D12_VIEWPORT{ 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
	m_scissorRect = { 0, 0, (long)width, (long)height };
	m_initialized = true;


	return true;
}

bool SilhouetteRenderPass::PopulateCommands(World* world, GraphicsContext* graphicsContext)
{
	if (world == nullptr || graphicsContext == nullptr)
	{
		return false;
	}

	LightingPass* dependencyLightingPass = (LightingPass*)GetDependencyPassOfType(PassType::LIGHTING_PASS);
	if (!dependencyLightingPass)
	{
		return false;
	}

	PassBase::PopulateCommands(world, graphicsContext);


	ID3D12GraphicsCommandList* commandList = m_commandBuilder->GetCommandList();
	DescriptorHeapManager* descHeapManager = graphicsContext->GetDescriptorHeapManager();

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;
	descHeapManager->GetDepthStencilViewCpuHandle(m_dsvId, dsvHandle);

	// step 1. draw depth
	// Set pso
	commandList->SetPipelineState(m_depthPipelineState->GetPipelineState());
	commandList->SetGraphicsRootSignature(m_depthPipelineState->GetRootSignature());

	ResourceBarrierTransition(m_depthStencilBuffer.Get(), commandList, D3D12_RESOURCE_STATE_DEPTH_WRITE);

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	commandList->OMSetRenderTargets(0, nullptr, false, &dsvHandle);
	commandList->RSSetViewports(1, &m_viewport);
	commandList->RSSetScissorRects(1, &m_scissorRect);

	// Bind uniform frame constant buffer
	D3D12_GPU_DESCRIPTOR_HANDLE uniformFrameGpuHandle;

	world->GetUniformFrameConstantBuffer()->BindConstantBufferViewToPipeline(graphicsContext, uniformFrameGpuHandle);
	commandList->SetGraphicsRootDescriptorTable(0, uniformFrameGpuHandle);

	for (auto& staticMesh : world->GetAllStaticMeshes())
	{
		staticMesh->Draw(graphicsContext, commandList, m_passType, false, false);
	}

	// step 2. draw stencil
	commandList->SetPipelineState(m_stencilPipelineState->GetPipelineState());
	commandList->SetGraphicsRootSignature(m_stencilPipelineState->GetRootSignature());
	commandList->SetGraphicsRootDescriptorTable(0, uniformFrameGpuHandle);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->OMSetRenderTargets(0, nullptr, false, &dsvHandle);
	commandList->RSSetViewports(1, &m_viewport);
	commandList->RSSetScissorRects(1, &m_scissorRect);
	commandList->OMSetStencilRef(1);
	for (auto& staticMesh : world->GetAllStaticMeshes())
	{
		if (!staticMesh->IsSelected())
		{
			continue;
		}
		staticMesh->Draw(graphicsContext, commandList, m_passType, false, false);
	}

	// step 3. draw silhouette
	commandList->SetPipelineState(m_silhouettePipelineState->GetPipelineState());
	commandList->SetGraphicsRootSignature(m_silhouettePipelineState->GetRootSignature());
	commandList->SetGraphicsRootDescriptorTable(0, uniformFrameGpuHandle);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
	descHeapManager->GetRenderTargetViewCpuHandle(dependencyLightingPass->GetRenderTargetBufferRtvId(), rtvHandle);
	dependencyLightingPass->RenderTargetBufferBarrierTransition(commandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);
	for (auto& staticMesh : world->GetAllStaticMeshes())
	{
		if (!staticMesh->IsSelected())
		{
			continue;
		}
		staticMesh->Draw(graphicsContext, commandList, m_passType, false, false);
	}

	dependencyLightingPass->RenderTargetBufferBarrierTransition(commandList, D3D12_RESOURCE_STATE_COPY_SOURCE);

	commandList->Close();
	return true;
}