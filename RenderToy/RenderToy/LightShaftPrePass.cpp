#include "LightShaftPrePass.h"
#include "Macros.h"
#include "Lights/SpotLight.h"

LightShaftPrePass::LightShaftPrePass(GUID passGuid)
	: RenderPassBase(passGuid)
{
	m_passType = PassType::LIGHT_SHAFT_PRE_PASS;
}

LightShaftPrePass::~LightShaftPrePass() {}

// Interface
bool LightShaftPrePass::Initialize(GraphicsContext* graphicsContext, ShaderManager* shaderManager)
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

	ID3D12Device* pDevice = graphicsContext->GetDevice();
	UINT adapterNodeMask = graphicsContext->GetAdapterNodeMask();
	DescriptorHeapManager* descHeapManager = graphicsContext->GetDescriptorHeapManager();

	D3D12_RESOURCE_DESC depthResourceDesc = {};
	depthResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthResourceDesc.Alignment = 0;
	depthResourceDesc.Width = width;
	depthResourceDesc.Height = height;
	depthResourceDesc.DepthOrArraySize = 1;
	depthResourceDesc.MipLevels = 1;
	depthResourceDesc.SampleDesc.Count = 1;
	depthResourceDesc.SampleDesc.Quality = 0;
	depthResourceDesc.Format = m_depthFormat;
	depthResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	D3D12_HEAP_PROPERTIES defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT, adapterNodeMask);
	if (FAILED(pDevice->CreateCommittedResource(
		&defaultHeap,
		D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
		&depthResourceDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(m_pDepthResource.GetAddressOf()))))
	{
		return false;
	}

	m_resourceStates[m_pDepthResource.Get()] = D3D12_RESOURCE_STATE_COMMON;

	m_depthUavId = descHeapManager->CreateUnorderedAccessView(m_pDepthResource.Get(), nullptr, nullptr);
	if (m_depthUavId == UINT64_MAX)
	{
		return false;
	}



	m_graphicsPipelineState = std::unique_ptr<GraphicsPipelineState>(new GraphicsPipelineState());
	D3D12_GRAPHICS_PIPELINE_STATE_DESC& pipelineStateDesc = m_graphicsPipelineState->GraphicsPipelineStateDesc();

	pipelineStateDesc.NodeMask = adapterNodeMask;
	pipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);;
	pipelineStateDesc.NumRenderTargets = 0;

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {
		FALSE,
		D3D12_DEPTH_WRITE_MASK_ZERO,
		D3D12_COMPARISON_FUNC_NONE,
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

	D3D12_INPUT_ELEMENT_DESC meshInputLayout[] = IA_MESH_SIMPLE_LAYOUT;

	pipelineStateDesc.RasterizerState = rasterizerDesc;
	pipelineStateDesc.DepthStencilState = depthStencilDesc;
	pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	pipelineStateDesc.SampleDesc.Count = 1;
	pipelineStateDesc.SampleDesc.Quality = 0;
	pipelineStateDesc.SampleMask = UINT_MAX;
	pipelineStateDesc.InputLayout.NumElements = NUM_IA_MESH_SIMPLE_LAYOUT;
	pipelineStateDesc.InputLayout.pInputElementDescs = meshInputLayout;

	if (!m_graphicsPipelineState->Initialize(graphicsContext, shaderManager, ShaderType::LIGHT_SHAFT_PRE_PASS_ROOT_SIGNATURE, ShaderType::LIGHT_SHAFT_PRE_PASS_VERTEX_SHADER, ShaderType::SHADER_TYPE_NONE, ShaderType::LIGHT_SHAFT_PRE_PASS_PIXEL_SHADER))
	{
		return false;
	}

	m_viewport = D3D12_VIEWPORT{ 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
	m_scissorRect = { 0, 0, (long)width, (long)height };


	return true;
}

bool LightShaftPrePass::PopulateCommands(World* world, GraphicsContext* graphicsContext)
{
	if (world == nullptr || graphicsContext == nullptr)
	{
		return false;
	}


	PassBase::PopulateCommands(world, graphicsContext);

	ID3D12GraphicsCommandList* commandList = m_commandBuilder->GetCommandList();

	DescriptorHeapManager* descHeapManager = graphicsContext->GetDescriptorHeapManager();
	ID3D12DescriptorHeap* descHeap = descHeapManager->GetCbvSrvUavShaderVisibleRingBufferHeap();

	// Set pso
	commandList->SetPipelineState(m_graphicsPipelineState->GetPipelineState());
	commandList->SetGraphicsRootSignature(m_graphicsPipelineState->GetRootSignature());
	commandList->SetDescriptorHeaps(1, &descHeap);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->RSSetViewports(1, &m_viewport);
	commandList->RSSetScissorRects(1, &m_scissorRect);

	// Transit the render target buffer to copy src state, since this might be copied out as final render result.
	ResourceBarrierTransition(m_pDepthResource.Get(), commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	D3D12_GPU_DESCRIPTOR_HANDLE depthGpuHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE depthCpuHandle;
	descHeapManager->BindCbvSrvUavToPipeline(m_depthUavId, depthGpuHandle);
	descHeapManager->GetCbvSrvUavNonShaderVisibleView(m_depthUavId, depthCpuHandle);
	commandList->SetGraphicsRootDescriptorTable(2, depthGpuHandle);
	float clearVal[4] = { 1.0f, 0.0f, 0.0f, 0.0f };
	commandList->ClearUnorderedAccessViewFloat(depthGpuHandle, depthCpuHandle, m_pDepthResource.Get(), clearVal, 0, nullptr);

	// Bind uniform frame constant buffer
	D3D12_GPU_DESCRIPTOR_HANDLE uniformFrameGpuHandle;
	world->GetUniformFrameConstantBuffer()->BindConstantBufferViewToPipeline(graphicsContext, uniformFrameGpuHandle);
	commandList->SetGraphicsRootDescriptorTable(0, uniformFrameGpuHandle);

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

				spotLight->DrawEffectiveFrustum(graphicsContext, commandList, transform.Translation);

				lightItr++;
			}
		}
	}

	m_commandBuilder->Close();

	return true;
}