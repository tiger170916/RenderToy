#include "RenderPipeline.h"
#include "PipelineOutputs.h"

RenderPipeline::RenderPipeline() 
{
}

bool RenderPipeline::Initialize(GraphicsContext* graphicsContext, ShaderManager* shaderManager)
{
	if (m_initialized)
	{
		return true;
	}

	// Create pipeline resource states recorder
	m_pipelineResourceStates = std::unique_ptr<PipelineResourceStates>(new PipelineResourceStates());

	// Initialize render passes
	m_earlyZPass = std::unique_ptr<EarlyZPass>(new EarlyZPass());
	if (!m_earlyZPass->Initialize(graphicsContext, shaderManager, m_pipelineResourceStates.get()))
	{
		return false;
	}

	m_geometryPass = std::unique_ptr<GeometryPass>(new GeometryPass());
	if (!m_geometryPass->Initialize(graphicsContext, shaderManager, m_pipelineResourceStates.get()))
	{
		return false;
	}

	m_commandQueue = std::unique_ptr<CommandQueue>(new CommandQueue(D3D12_COMMAND_QUEUE_FLAG_NONE, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_QUEUE_PRIORITY_NORMAL, graphicsContext->GetAdapterNodeMask()));
	if (!m_commandQueue->Initialize(graphicsContext->GetDevice()))
	{
		return false;
	}

	m_commandBuilder = std::unique_ptr<CommandBuilder>(new CommandBuilder(D3D12_COMMAND_LIST_TYPE_DIRECT));
	if (!m_commandBuilder->Initialize(graphicsContext->GetDevice()))
	{
		return false;
	}

	m_initialized = true;
	return true;
}

void RenderPipeline::FrameBegin()
{
	m_commandBuilder->Reset();
	ID3D12GraphicsCommandList* commandList = m_commandBuilder->GetCommandList();
}

void RenderPipeline::Frame(GraphicsContext* graphicsContext, World* worldToRender)
{
	ID3D12GraphicsCommandList* cmdList = m_commandBuilder->GetCommandList();

	// Bind the global ring descriptor heap to the commandlist.
	ID3D12DescriptorHeap* cbvSrvUavDescHeap = graphicsContext->GetDescriptorHeapManager()->GetCbvSrvUavShaderVisibleRingBufferHeap();
	cmdList->SetDescriptorHeaps(1, &cbvSrvUavDescHeap);
	
	PipelineOutputsStruct outputs = {};

	m_earlyZPass->Frame(worldToRender, cmdList, graphicsContext, m_pipelineResourceStates.get(), outputs);
	m_geometryPass->Frame(worldToRender, cmdList, graphicsContext, m_pipelineResourceStates.get(), outputs);
}

void RenderPipeline::FrameEnd()
{
	m_commandBuilder->Close();
	m_commandQueue->DispatchCommands(m_commandBuilder.get());
	m_commandQueue->SignalAndWait();
}


RenderPipeline::~RenderPipeline()
{

}