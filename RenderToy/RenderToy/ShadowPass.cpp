#include "ShadowPass.h"
#include "Macros.h"
#include "GraphicsUtils.h"
#include "Lights/SpotLight.h"
#include "Lights/PointLight.h"

ShadowPass::ShadowPass(GUID passGuid)
	: RenderPassBase(passGuid)
{
	m_passType = PassType::SHADOW_PASS;
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

	m_resourceStates[m_atlas->GetResource()] = D3D12_RESOURCE_STATE_COMMON;

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

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {
	TRUE,
	D3D12_DEPTH_WRITE_MASK_ALL,
	D3D12_COMPARISON_FUNC_LESS_EQUAL,
	FALSE
	};

	pipelineStateDesc.RasterizerState = rasterizerDesc;
	pipelineStateDesc.DepthStencilState = depthStencilDesc;
	pipelineStateDesc.DSVFormat = m_rtDepthFormat;
	pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateDesc.SampleDesc.Count = 1;
	pipelineStateDesc.SampleDesc.Quality = 0;
	pipelineStateDesc.SampleMask = UINT_MAX;
	pipelineStateDesc.InputLayout.NumElements = NUM_IA_MESH_SIMPLE_LAYOUT;
	pipelineStateDesc.InputLayout.pInputElementDescs = meshInputLayout;
	pipelineStateDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	if (!m_graphicsPipelineState->Initialize(graphicsContext, shaderManager, ShaderType::SHADOW_PASS_ROOT_SIGNATURE, ShaderType::SHADOW_PASS_VERTEX_SHADER, ShaderType::SHADOW_PASS_GEOMETRY_SHADER, ShaderType::SHADOW_PASS_PIXEL_SHADER))
	{
		return false;
	}

	m_viewports[0] = D3D12_VIEWPORT{0.0f, 0.0f, (float)m_l1ShadowMapSize, (float)m_l1ShadowMapSize, 0.0f, 1.0f};
	m_scissorRects[0] = {0, 0, (long)m_l1ShadowMapSize, (long)m_l1ShadowMapSize};

	m_viewports[1] = D3D12_VIEWPORT{ 0.0f, 0.0f, (float)m_l2ShadowMapSize, (float)m_l2ShadowMapSize, 0.0f, 1.0f };
	m_scissorRects[1] = { 0, 0, (long)m_l2ShadowMapSize, (long)m_l2ShadowMapSize };

	if (!GraphicsUtils::CreateDepthStencilResource(
		pDevice,
		descHeapManager,
		m_l1ShadowMapSize,
		m_l1ShadowMapSize,
		m_rtDepthFormat,
		D3D12_RESOURCE_FLAG_NONE,
		1.0f,
		0,
		m_depthStencilBuffers[0].GetAddressOf(),
		m_dsvIds[0]))
	{
		return false;
	}

	m_resourceStates[m_depthStencilBuffers[0].Get()] = D3D12_RESOURCE_STATE_DEPTH_WRITE;

	if (!GraphicsUtils::CreateDepthStencilResource(
		pDevice,
		descHeapManager,
		m_l1ShadowMapSize,
		m_l1ShadowMapSize,
		m_rtDepthFormat,
		D3D12_RESOURCE_FLAG_NONE,
		1.0f,
		0,
		m_depthStencilBuffers[1].GetAddressOf(),
		m_dsvIds[1]))
	{
		return false;
	}

	m_resourceStates[m_depthStencilBuffers[1].Get()] = D3D12_RESOURCE_STATE_DEPTH_WRITE;

	return true;
}

bool ShadowPass::PopulateCommands(World* world, GraphicsContext* graphicsContext)
{


	if (world == nullptr || graphicsContext == nullptr)
	{
		return false;
	}

	DescriptorHeapManager* descriptorHeapManager = graphicsContext->GetDescriptorHeapManager();
	ConstantBuffer<LightConstantsDx>* lightCb = world->GetLightConstantBuffer();

	m_atlas->ClearNodes();
	std::vector<TextureAtlas::Node> nodes;

	UINT lightItr = 0;
	for (auto& staticMesh : world->GetAllStaticMeshes())
	{
		if (!staticMesh->HasLightExtensions())
		{
			continue;
		}

		for (uint32_t instanceIdx = 0; instanceIdx < staticMesh->GetNumInstances(); instanceIdx++)
		{
			if (!staticMesh->HasLightExtension(instanceIdx))
			{
				continue;
			}

			LightExtension* light = staticMesh->GetLightExtension(instanceIdx);
			if (!light)
			{
				continue;
			}

			Transform transform;
			staticMesh->GetInstanceTransform(instanceIdx, transform);

			uint32_t instanceUid;
			staticMesh->GetInstanceUid(instanceIdx, instanceUid);

			if (light->GetLightType() == LightType::LightType_Spot)
			{
				SpotLight* spotLight = (SpotLight*)light;

				const FVector3& intensity = light->GetIntensity();
				const FVector3& position = light->GetPosition() + transform.Translation;

				TextureAtlas::Node node;
				m_atlas->RequestNode(1, nullptr, node);
				nodes.push_back(node);

				LightConstants lightConsts = {};
				lightConsts.ShadowBufferOffsetX = node.OffsetX;
				lightConsts.ShadowBufferOffsetY = node.OffsetY;
				lightConsts.Intensity[0] = intensity.X;
				lightConsts.Intensity[1] = intensity.Y;
				lightConsts.Intensity[2] = intensity.Z;
				lightConsts.LightType = (uint32_t)light->GetLightType();
				lightConsts.Position[0] = position.X;
				lightConsts.Position[1] = position.Y;
				lightConsts.Position[2] = position.Z;
				lightConsts.ShadowBufferSize = m_l1ShadowMapSize;
				lightConsts.LightParentUid = instanceUid;
				lightConsts.LightUid = light->GetUid();

				(*lightCb)[0].Lights[lightItr] = lightConsts;

				XMMATRIX view = GraphicsUtils::ViewMatrixFromPositionRotation(spotLight->GetPosition() + transform.Translation, spotLight->GetRotator());
				DirectX::XMStoreFloat4x4(&(*lightCb)[0].Lights[lightItr].Transform, DirectX::XMMatrixTranspose(spotLight->GetProjectionMatrix()) * DirectX::XMMatrixTranspose(view));
				lightItr++;
			}
		}
	}


	// No light
	if (lightItr == 0)
	{
		return true;
	}

	(*lightCb)[0].NumLights[0] = lightItr;
	lightCb->UpdateToGPU();

	PassBase::PopulateCommands(world, graphicsContext);

	ID3D12GraphicsCommandList* commandList = m_commandBuilder->GetCommandList();

	float clearValue[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	D3D12_GPU_DESCRIPTOR_HANDLE atlasGpuHandle;
	descriptorHeapManager->BindCbvSrvUavToPipeline(m_depthAtlasUavId, atlasGpuHandle);
	D3D12_CPU_DESCRIPTOR_HANDLE atlasNonShaderVisibleCpuHandle;
	descriptorHeapManager->GetCbvSrvUavNonShaderVisibleView(m_depthAtlasUavId, atlasNonShaderVisibleCpuHandle);

	D3D12_GPU_DESCRIPTOR_HANDLE lightConstantsGpuDescHandle;
	commandList->SetPipelineState(m_graphicsPipelineState->GetPipelineState());
	commandList->SetGraphicsRootSignature(m_graphicsPipelineState->GetRootSignature());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3D12_CPU_DESCRIPTOR_HANDLE dsvhandle;
	descriptorHeapManager->GetDepthStencilViewCpuHandle(m_dsvIds[0], dsvhandle);
	commandList->OMSetRenderTargets(0, nullptr, false, &dsvhandle);
	commandList->ClearDepthStencilView(dsvhandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	ResourceBarrierTransition(m_atlas->GetResource(), commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandList->ClearUnorderedAccessViewFloat(atlasGpuHandle, atlasNonShaderVisibleCpuHandle, m_atlas->GetResource(), clearValue, 0, nullptr);

	lightCb->BindConstantBufferViewToPipeline(graphicsContext, lightConstantsGpuDescHandle);
	commandList->SetGraphicsRootDescriptorTable(1, lightConstantsGpuDescHandle);
	commandList->RSSetViewports(1, m_viewports);
	commandList->RSSetScissorRects(1, m_scissorRects);

	commandList->SetGraphicsRootDescriptorTable(2, atlasGpuHandle);

	for (auto& staticMesh : world->GetAllStaticMeshes())
	{
		if (!staticMesh->PassEnabled(PassType::SHADOW_PASS))
		{
			continue;
		}

		staticMesh->Draw(graphicsContext, commandList, m_passType, true, false);
	}

	m_commandBuilder->Close();

	return true;
}