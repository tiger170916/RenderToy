#include "LightingPass.h"
#include "GeometryPass.h"
#include "ShadowPass.h"
#include "GraphicsUtils.h"
#include "MeshFactory.h"
#include "Macros.h"

LightingPass::LightingPass(GUID passGuid)
	: RenderPassBase(passGuid)
{
	m_passType = PassType::LIGHTING_PASS;
}

LightingPass::~LightingPass()
{

}

bool LightingPass::Initialize(GraphicsContext* graphicsContext, ShaderManager* shaderManager)
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

	if (!GraphicsUtils::CreateRenderTargetResource(
		pDevice,
		descHeapManager,
		width,
		height,
		m_renderTargetFormat,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		m_renderTargetClearValue,
		m_renderTarget.GetAddressOf(),
		m_rtvId))
	{
		return false;
	}

	m_renderTarget->SetName(L"Final render target");

	m_resourceStates[m_renderTarget.Get()] = D3D12_RESOURCE_STATE_PRESENT;

	m_uavId = descHeapManager->CreateUnorderedAccessView(m_renderTarget.Get(), nullptr, nullptr);
	if (m_uavId == UINT64_MAX)
	{
		return false;
	}

	m_graphicsPipelineState = std::unique_ptr<GraphicsPipelineState>(new GraphicsPipelineState());
	D3D12_GRAPHICS_PIPELINE_STATE_DESC& pipelineStateDesc = m_graphicsPipelineState->GraphicsPipelineStateDesc();

	pipelineStateDesc.NodeMask = adapterNodeMask;
	pipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);;
	pipelineStateDesc.NumRenderTargets = 1;
	pipelineStateDesc.RTVFormats[0] = m_renderTargetFormat;

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

	D3D12_INPUT_ELEMENT_DESC meshInputLayout[] = IA_MESH_LAYOUT;

	pipelineStateDesc.RasterizerState = rasterizerDesc;
	pipelineStateDesc.DepthStencilState = depthStencilDesc;
	pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	pipelineStateDesc.SampleDesc.Count = 1;
	pipelineStateDesc.SampleDesc.Quality = 0;
	pipelineStateDesc.SampleMask = UINT_MAX;
	pipelineStateDesc.InputLayout.NumElements = NUM_IA_MESH_LAYOUT;
	pipelineStateDesc.InputLayout.pInputElementDescs = meshInputLayout;

	if (!m_graphicsPipelineState->Initialize(graphicsContext, shaderManager, ShaderType::LIGHTING_PASS_ROOT_SIGNATURE, ShaderType::LIGHTING_PASS_VERTEX_SHADER, ShaderType::SHADER_TYPE_NONE, ShaderType::LIGHTING_PASS_PIXEL_SHADER))
	{
		return false;
	}

	m_viewport = D3D12_VIEWPORT{ 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
	m_scissorRect = { 0, 0, (long)width, (long)height };

	m_rectangleMesh = std::unique_ptr<StaticMesh>(MeshFactory::Get()->CreateStaticMesh());
	StaticMesh::MeshVertex point1
	{
		.Position = FVector3(-1.0f, -1.0f, 0.1f),
	};
	StaticMesh::MeshVertex point2
	{
		.Position = FVector3(1.0f, -1.0f, 0.1f),
	};
	StaticMesh::MeshVertex point3
	{
		.Position = FVector3(1.0f, 1.0f, 0.1f),
	};

	StaticMesh::MeshVertex point4
	{
		.Position = FVector3(-1.0f, 1.0f, 0.1f),
	};

	m_rectangleMesh->AddTriangle(-1, point1, point3, point2);
	m_rectangleMesh->AddTriangle(-1, point1, point4, point3);

	// Add an instance with trival transform, since this info is not gonna used.
	MeshFactory::Get()->StaticMeshAddInstance(m_rectangleMesh.get(), Transform::Identity());
	if (!m_rectangleMesh->BuildResource(graphicsContext, nullptr))
	{
		return false;
	}

	D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_RESOURCE_DESC testResourceDesc = {};
	testResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	testResourceDesc.Alignment = 0;
	testResourceDesc.Width = graphicsContext->GetHwndWidth();
	testResourceDesc.Height = graphicsContext->GetHwndHeight();
	testResourceDesc.DepthOrArraySize = 1;
	testResourceDesc.MipLevels = 1;
	testResourceDesc.SampleDesc.Count = 1;
	testResourceDesc.SampleDesc.Quality = 0;
	testResourceDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	testResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	testResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	if (FAILED(pDevice->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
		&testResourceDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(m_pTestResource.GetAddressOf()))))
	{
		return false;
	}

	m_resourceStates[m_pTestResource.Get()] = D3D12_RESOURCE_STATE_COMMON;
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	uavDesc.Texture2D.PlaneSlice = 0;

	m_testResourceUavId = descHeapManager->CreateUnorderedAccessView(m_pTestResource.Get(), nullptr, &uavDesc);

	m_initialized = true;

	return true;
}

bool LightingPass::PopulateCommands(World* world, GraphicsContext* graphicsContext)
{
	if (world == nullptr || graphicsContext == nullptr)
	{
		return false;
	}

	PassBase::PopulateCommands(world, graphicsContext);

	ID3D12GraphicsCommandList* commandList = m_commandBuilder->GetCommandList();

	DescriptorHeapManager* descHeapManager = graphicsContext->GetDescriptorHeapManager();
	D3D12_CPU_DESCRIPTOR_HANDLE rtv;
	descHeapManager->GetRenderTargetViewCpuHandle(m_rtvId, rtv);

	ResourceBarrierTransition(m_renderTarget.Get(), commandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
	// Set pso
	commandList->SetPipelineState(m_graphicsPipelineState->GetPipelineState());
	commandList->SetGraphicsRootSignature(m_graphicsPipelineState->GetRootSignature());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->ClearRenderTargetView(rtv, m_renderTargetClearValue, 0, NULL);
	commandList->OMSetRenderTargets(1, &rtv, false, nullptr);
	commandList->RSSetViewports(1, &m_viewport);
	commandList->RSSetScissorRects(1, &m_scissorRect);
	

	ResourceBarrierTransition(m_pTestResource.Get(), commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	D3D12_GPU_DESCRIPTOR_HANDLE testGpuHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE testCpuHandle;
	descHeapManager->BindCbvSrvUavToPipeline(m_testResourceUavId, testGpuHandle);
	descHeapManager->GetCbvSrvUavNonShaderVisibleView(m_testResourceUavId, testCpuHandle);
	commandList->SetGraphicsRootDescriptorTable(9, testGpuHandle);
	float clearVal[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	commandList->ClearUnorderedAccessViewFloat(testGpuHandle, testCpuHandle, m_pTestResource.Get(), clearVal, 0, nullptr);


	// Bind uniform frame constant buffer
	D3D12_GPU_DESCRIPTOR_HANDLE uniformFrameGpuHandle;

	world->GetUniformFrameConstantBuffer()->BindConstantBufferViewToPipeline(graphicsContext, uniformFrameGpuHandle);
	commandList->SetGraphicsRootDescriptorTable(0, uniformFrameGpuHandle);

	GeometryPass* dependencyGeometryPass = (GeometryPass*)GetDependencyPassOfType(PassType::GEOMETRY_PASS);
	ShadowPass* dependencyShadowPass = (ShadowPass*)GetDependencyPassOfType(PassType::SHADOW_PASS);
	
	if (dependencyGeometryPass)
	{
		dependencyGeometryPass->DiffuseBufferBarrierTransition(commandList, D3D12_RESOURCE_STATE_COMMON);
		dependencyGeometryPass->MetallicRoughnessBufferBarrierTransition(commandList, D3D12_RESOURCE_STATE_COMMON);
		dependencyGeometryPass->NormalBufferBarrierTransition(commandList, D3D12_RESOURCE_STATE_COMMON);
		dependencyGeometryPass->WorldPosBufferBarrierTransition(commandList, D3D12_RESOURCE_STATE_COMMON);
		dependencyGeometryPass->WorldEmissionBufferBarrierTransition(commandList, D3D12_RESOURCE_STATE_COMMON);

		D3D12_GPU_DESCRIPTOR_HANDLE geometryPassDiffuseBufferHandle, geometryPassMetallicRoughnessBufferHandle, geometryPassNormalBufferHandle, geometryPassWorldPosBufferHandle, geometryPassEmissionBufferHandle;
		descHeapManager->BindCbvSrvUavToPipeline(dependencyGeometryPass->GetDiffuseBufferSrvId(), geometryPassDiffuseBufferHandle);
		descHeapManager->BindCbvSrvUavToPipeline(dependencyGeometryPass->GetMetallicRoughnessBufferSrvId(), geometryPassMetallicRoughnessBufferHandle);
		descHeapManager->BindCbvSrvUavToPipeline(dependencyGeometryPass->GetNormalBufferSrvId(), geometryPassNormalBufferHandle);
		descHeapManager->BindCbvSrvUavToPipeline(dependencyGeometryPass->GetWorldPosBufferSrvId(), geometryPassWorldPosBufferHandle);
		descHeapManager->BindCbvSrvUavToPipeline(dependencyGeometryPass->GetEmissionBufferSrvId(), geometryPassEmissionBufferHandle);

		commandList->SetGraphicsRootDescriptorTable(3, geometryPassDiffuseBufferHandle);
		commandList->SetGraphicsRootDescriptorTable(4, geometryPassMetallicRoughnessBufferHandle);
		commandList->SetGraphicsRootDescriptorTable(5, geometryPassNormalBufferHandle);
		commandList->SetGraphicsRootDescriptorTable(6, geometryPassWorldPosBufferHandle);
		commandList->SetGraphicsRootDescriptorTable(7, geometryPassEmissionBufferHandle);
	}

	if (dependencyShadowPass)
	{
		dependencyShadowPass->DepthAtlasBarrierTransition(commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		D3D12_GPU_DESCRIPTOR_HANDLE shadowMapAltasGpuHandle;
		descHeapManager->BindCbvSrvUavToPipeline(dependencyShadowPass->GetDepthAtlasUavId(), shadowMapAltasGpuHandle);

		commandList->SetGraphicsRootDescriptorTable(8, shadowMapAltasGpuHandle);
	}

	ConstantBuffer<LightConstantsDx>* lightsCb = world->GetLightConstantBuffer();
	if (lightsCb)
	{
		D3D12_GPU_DESCRIPTOR_HANDLE lightsCbGpuHandle;
		lightsCb->BindConstantBufferViewToPipeline(graphicsContext, lightsCbGpuHandle);

		commandList->SetGraphicsRootDescriptorTable(2, lightsCbGpuHandle);
	}

	// Draw fullscreen rectangle
	m_rectangleMesh->Draw(graphicsContext, commandList, m_passType, false, false);

	// Transit the render target buffer to copy src state, since this might be copied out as final render result.
	ResourceBarrierTransition(m_renderTarget.Get(), commandList, D3D12_RESOURCE_STATE_COPY_SOURCE);

	m_commandBuilder->Close();

	return true;
}

ID3D12Resource* LightingPass::GetFinalRenderPassOutputResource() const
{
	return m_renderTarget.Get();
}