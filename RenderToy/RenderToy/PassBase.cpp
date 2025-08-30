#include "PassBase.h"
#include "GraphicsUtils.h"

PassBase::PassBase(GUID passGuid)
	: m_passGuid(passGuid)
{

}

PassBase::~PassBase()
{

}

bool PassBase::Initialize(GraphicsContext* graphicsContext, ShaderManager* shaderManager)
{
	if (!graphicsContext || !shaderManager)
	{
		return false;
	}

	m_commandBuilder = std::unique_ptr<CommandBuilder>(new CommandBuilder(D3D12_COMMAND_LIST_TYPE_DIRECT));
	if (!m_commandBuilder->Initialize(graphicsContext->GetDevice()))
	{
		return false;
	}

	m_fence = std::unique_ptr<D3DFence>(new D3DFence());
	if (!m_fence->Initialize(graphicsContext->GetDevice()))
	{
		return false;
	}

	return true;
}

void PassBase::AddDependency(PassBase* dependencyPass)
{
	if (dependencyPass == nullptr)
	{
		return;
	}

	bool found = false;
	for (auto& dependency : m_dependencyPasses)
	{
		if (dependency == dependencyPass)
		{
			found = true;
			break;
		}
	}

	if (!found)
	{
		m_dependencyPasses.push_back(dependencyPass);
	}

	found = false;
	for (auto& dependency : dependencyPass->m_dependentPasses)
	{
		if (dependency == dependencyPass)
		{
			found = true;
			break;
		}
	}

	if (!found)
	{
		dependencyPass->m_dependentPasses.push_back(this);
	}
}

bool PassBase::ResourceBarrierTransition(ID3D12Resource* resource, ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES stateAfter)
{
	if (!resource || !m_resourceStates.contains(resource))
	{
		return false;
	}

	D3D12_RESOURCE_STATES stateBefore = m_resourceStates[resource];
	GraphicsUtils::ResourceBarrierTransition(resource, commandList, stateBefore, stateAfter);
	m_resourceStates[resource] = stateAfter;

	return true;
}

PassBase* PassBase::GetDependencyPassOfType(PassType passType)
{
	for (int i = 0; i < m_dependencyPasses.size(); i++)
	{
		if (m_dependencyPasses[i] == nullptr) 
		{
			continue;
		}

		if (m_dependencyPasses[i]->GetPassType() == passType)
		{
			return m_dependencyPasses[i];
		}
	}

	return nullptr;
}

bool PassBase::PopulateCommands(World* world, GraphicsContext* graphicsContext)
{
	if (!graphicsContext)
	{
		return false;
	}

	m_commandBuilder->Reset();
	ID3D12DescriptorHeap* cbvSrvUavDescriptorHeap = graphicsContext->GetDescriptorHeapManager()->GetCbvSrvUavShaderVisibleRingBufferHeap();

	ID3D12GraphicsCommandList* commandList = m_commandBuilder->GetCommandList();
	commandList->SetDescriptorHeaps(1, &cbvSrvUavDescriptorHeap);

	return true;
}

bool PassBase::UpdateBuffers(World* world)
{
	return true;
}

ID3D12Resource* PassBase::GetFinalRenderPassOutputResource() const
{
	return nullptr;
}