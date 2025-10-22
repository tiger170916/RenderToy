#include "LightShaftPass.h"
#include "GeometryPass.h"
#include "LightingPass.h"
#include "LightShaftPrePass.h"
#include "ShadowPass.h"
#include "FullscreenQuad.h"
#include "Macros.h"

LightShaftPass::LightShaftPass(GUID passGuid)
	: RenderPassBase(passGuid)
{

}

LightShaftPass::~LightShaftPass()
{

}

bool LightShaftPass::Initialize(GraphicsContext* graphicsContext, ShaderManager* shaderManager)
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

	const UINT& width = graphicsContext->GetHwndWidth();
	const UINT& height = graphicsContext->GetHwndHeight();
	const UINT& adapterNodeMask = graphicsContext->GetAdapterNodeMask();

	m_graphicsPipelineState = std::unique_ptr<GraphicsPipelineState>(new GraphicsPipelineState());
	D3D12_GRAPHICS_PIPELINE_STATE_DESC& pipelineStateDesc = m_graphicsPipelineState->GraphicsPipelineStateDesc();

	pipelineStateDesc.NodeMask = adapterNodeMask;
	pipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);;
	pipelineStateDesc.NumRenderTargets = 0;

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

	D3D12_INPUT_ELEMENT_DESC meshInputLayout[] = IA_FULLSCREEN_QUAD_LAYOUT;

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {
	FALSE,
	D3D12_DEPTH_WRITE_MASK_ALL,
	D3D12_COMPARISON_FUNC_LESS_EQUAL,
	FALSE
	};

	pipelineStateDesc.RasterizerState = rasterizerDesc;

	pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateDesc.SampleDesc.Count = 1;
	pipelineStateDesc.SampleDesc.Quality = 0;
	pipelineStateDesc.SampleMask = UINT_MAX;
	pipelineStateDesc.InputLayout.NumElements = NUM_IA_FULLSCREEN_QUAD_LAYOUT;
	pipelineStateDesc.InputLayout.pInputElementDescs = meshInputLayout;
	pipelineStateDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	if (!m_graphicsPipelineState->Initialize(graphicsContext, shaderManager, ShaderType::LIGHT_SHAFT_PASS_ROOT_SIGNATURE, ShaderType::LIGHT_SHAFT_PASS_VERTEX_SHADER, ShaderType::SHADER_TYPE_NONE, ShaderType::LIGHT_SHAFT_PASS_PIXEL_SHADER))
	{
		return false;
	}

	m_viewport = D3D12_VIEWPORT{ 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
	m_scissorRect = { 0, 0, (long)width, (long)height };

	return true;
}

bool LightShaftPass::PopulateCommands(World* world, GraphicsContext* graphicsContext)
{
	if (world == nullptr || graphicsContext == nullptr)
	{
		return false;
	}

	DescriptorHeapManager* descHeapManager = graphicsContext->GetDescriptorHeapManager();

	PassBase::PopulateCommands(world, graphicsContext);

	ID3D12GraphicsCommandList* commandList = m_commandBuilder->GetCommandList();

	commandList->SetPipelineState(m_graphicsPipelineState->GetPipelineState());

	commandList->SetGraphicsRootSignature(m_graphicsPipelineState->GetRootSignature());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3D12_GPU_DESCRIPTOR_HANDLE uniformFrameGpuHandle;
	world->GetUniformFrameConstantBuffer()->BindConstantBufferViewToPipeline(graphicsContext, uniformFrameGpuHandle);
	commandList->SetGraphicsRootDescriptorTable(0, uniformFrameGpuHandle);

	GeometryPass* dependencyGeometryPass = (GeometryPass*)GetDependencyPassOfType(PassType::GEOMETRY_PASS);
	LightShaftPrePass* dependencyLightShaftPrePass = (LightShaftPrePass*)GetDependencyPassOfType(PassType::LIGHT_SHAFT_PRE_PASS);
	LightingPass* dependencyLightingPass = (LightingPass*)GetDependencyPassOfType(PassType::LIGHTING_PASS);
	ShadowPass* dependencyShadowPass = (ShadowPass*)GetDependencyPassOfType(PassType::SHADOW_PASS);

	if (dependencyGeometryPass && dependencyLightShaftPrePass && dependencyLightingPass && dependencyShadowPass)
	{
		D3D12_GPU_DESCRIPTOR_HANDLE lightingPassRenderTargetHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE geometryPassWorldPosBufferHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE lightShaftPrePassPositionBufferHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE shadowAtlasHandle;

		dependencyLightingPass->RenderTargetBufferBarrierTransition(commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		dependencyGeometryPass->WorldPosBufferBarrierTransition(commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		dependencyLightShaftPrePass->PositionBufferBarrierTransition(commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		dependencyShadowPass->DepthAtlasBarrierTransition(commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		descHeapManager->BindCbvSrvUavToPipeline(dependencyLightingPass->GetRenderTargetBufferUavId(), lightingPassRenderTargetHandle);
		descHeapManager->BindCbvSrvUavToPipeline(dependencyGeometryPass->GetWorldPosBufferUavId(), geometryPassWorldPosBufferHandle);
		descHeapManager->BindCbvSrvUavToPipeline(dependencyLightShaftPrePass->GetPositionBufferUavId(), lightShaftPrePassPositionBufferHandle);
		descHeapManager->BindCbvSrvUavToPipeline(dependencyShadowPass->GetDepthAtlasUavId(), shadowAtlasHandle);

		commandList->SetGraphicsRootDescriptorTable(2, lightingPassRenderTargetHandle);
		commandList->SetGraphicsRootDescriptorTable(3, geometryPassWorldPosBufferHandle);
		commandList->SetGraphicsRootDescriptorTable(4, lightShaftPrePassPositionBufferHandle);
		commandList->SetGraphicsRootDescriptorTable(5, shadowAtlasHandle);
	}

	ConstantBuffer<LightConstantsDx>* lightCb = world->GetLightConstantBuffer();
	D3D12_GPU_DESCRIPTOR_HANDLE lightConstantsGpuDescHandle;
	lightCb->BindConstantBufferViewToPipeline(graphicsContext, lightConstantsGpuDescHandle);
	commandList->SetGraphicsRootDescriptorTable(1, lightConstantsGpuDescHandle);

	commandList->OMSetRenderTargets(0, nullptr, false, nullptr);
	commandList->RSSetViewports(1, &m_viewport);
	commandList->RSSetScissorRects(1, &m_scissorRect);

	FullScreenQuad* fullscreenQuad = FullScreenQuad::Get();
	fullscreenQuad->Draw(commandList);

	dependencyLightingPass->RenderTargetBufferBarrierTransition(commandList, D3D12_RESOURCE_STATE_COPY_SOURCE);

	commandList->Close();

	return true;
}


bool LightShaftPass::PopulateCommands(World2* world, MaterialManager* materialManager, TextureManager2* textureManager, GraphicsContext* graphicsContext)
{
	if (world == nullptr || graphicsContext == nullptr)
	{
		return false;
	}

	PassBase::PopulateCommands(world, materialManager, textureManager, graphicsContext);

	ID3D12GraphicsCommandList* commandList = m_commandBuilder->GetCommandList();

	m_commandBuilder->Close();
	return true;
}