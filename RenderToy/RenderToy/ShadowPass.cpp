#include "ShadowPass.h"
#include "Macros.h"

ShadowPass::ShadowPass(GUID passGuid)
	: RenderPassBase(passGuid)
{
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
	UINT adapterNodeMask = graphicsContext->GetAdapterNodeMask();

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

	m_atlas = std::unique_ptr<TextureAtlas>(new TextureAtlas());
	if (!m_atlas->Initialize(graphicsContext, m_depthAtlasBufferSize, {m_l1ShadowMapSize, m_l2ShadowMapSize}, &depthAtlasResourceDesc))
	{
		return false;
	}

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = m_depthFormat;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	uavDesc.Texture2D.PlaneSlice = 0;

	m_depthAtlasUavId = descHeapManager->CreateUnorderedAccessView(m_atlas->GetResource(), nullptr, &uavDesc);

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

	D3D12_INPUT_ELEMENT_DESC meshInputLayout[] = IA_MESH_SIMPLE_LAYOUT;

	pipelineStateDesc.RasterizerState = rasterizerDesc;
	pipelineStateDesc.DepthStencilState.DepthEnable = FALSE;
	pipelineStateDesc.DepthStencilState.StencilEnable = FALSE;
	pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateDesc.SampleDesc.Count = 1;
	pipelineStateDesc.SampleDesc.Quality = 0;
	pipelineStateDesc.SampleMask = UINT_MAX;
	pipelineStateDesc.InputLayout.NumElements = NUM_IA_MESH_SIMPLE_LAYOUT;
	pipelineStateDesc.InputLayout.pInputElementDescs = meshInputLayout;

	if (!m_graphicsPipelineState->Initialize(graphicsContext, shaderManager, ShaderType::SHADOW_PASS_ROOT_SIGNATURE, ShaderType::SHADOW_PASS_VERTEX_SHADER, ShaderType::SHADOW_PASS_GEOMETRY_SHADER, ShaderType::SHADOW_PASS_PIXEL_SHADER))
	{
		return false;
	}

	m_viewports[0] = D3D12_VIEWPORT{0.0f, 0.0f, (float)m_l1ShadowMapSize, (float)m_l1ShadowMapSize, 0.0f, 1.0f};
	m_scissorRects[0] = {0, 0, (long)m_l1ShadowMapSize, (long)m_l1ShadowMapSize};

	m_viewports[1] = D3D12_VIEWPORT{ 0.0f, 0.0f, (float)m_l2ShadowMapSize, (float)m_l2ShadowMapSize, 0.0f, 1.0f };
	m_scissorRects[2] = { 0, 0, (long)m_l2ShadowMapSize, (long)m_l2ShadowMapSize };

	// Create light constants
	m_lightConstants = std::unique_ptr<ConstantBuffer<LightConstantsDx>>(new ConstantBuffer<LightConstantsDx>(m_maxNumLightSource));
	if (!m_lightConstants->Initialize(graphicsContext))
	{
		return false;
	}



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

	std::vector<Transform> allLights;

	for (auto& staticMesh : world->GetAllStaticMeshes())
	{
		if (!staticMesh->HasLightExtensions())
		{
			continue;
		}

		const std::vector<std::shared_ptr<LightExtension>>& lights = staticMesh->GetLightExtensions();
		for (auto& light : lights)
		{
			for (auto& instance : staticMesh->GetInstances())
			{
				allLights.push_back(instance);
			}
		}
	}

	m_commandBuilder->Close();

	return true;
}