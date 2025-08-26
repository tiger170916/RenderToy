#include "SilhouetteRenderPass.h"
#include "LightingPass.h"
#include "Macros.h"

SilhouetteRenderPass::SilhouetteRenderPass(GUID passGuid)
	: RenderPassBase(passGuid){}

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
	D3D12_RESOURCE_DESC depthStencilResourceDesc = {};
	depthStencilResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilResourceDesc.Alignment = 0;
	depthStencilResourceDesc.Width = width;
	depthStencilResourceDesc.Height = height;
	depthStencilResourceDesc.DepthOrArraySize = 1;
	depthStencilResourceDesc.MipLevels = 1;
	depthStencilResourceDesc.SampleDesc.Count = 1;
	depthStencilResourceDesc.SampleDesc.Quality = 0;
	depthStencilResourceDesc.Format = m_depthFormat;
	depthStencilResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear = {};
	optClear.Format = m_depthFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	if (FAILED(pDevice->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
		&depthStencilResourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&optClear,
		IID_PPV_ARGS(m_depthStencilBuffer.GetAddressOf()))))
	{
		return false;
	}

	D3D12_INPUT_ELEMENT_DESC meshInputLayout[] = IA_MESH_LAYOUT;

	m_depthPipelineState = std::unique_ptr<GraphicsPipelineState>(new GraphicsPipelineState());
	D3D12_GRAPHICS_PIPELINE_STATE_DESC& depthPipelineStateDesc = m_depthPipelineState->GraphicsPipelineStateDesc();
	depthPipelineStateDesc.NodeMask = adapterNodeMask;
	depthPipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	depthPipelineStateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	depthPipelineStateDesc.DepthStencilState.DepthEnable = true;
	depthPipelineStateDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthPipelineStateDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
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
	stencilPipelineStateDesc.DepthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
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
	silhouettePipelineStateDesc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_LESS;
	silhouettePipelineStateDesc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	silhouettePipelineStateDesc.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	silhouettePipelineStateDesc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	silhouettePipelineStateDesc.DepthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_LESS;
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

	PassBase::PopulateCommands(world, graphicsContext);


	ID3D12GraphicsCommandList* commandList = m_commandBuilder->GetCommandList();

	commandList->Close();
	return true;
}