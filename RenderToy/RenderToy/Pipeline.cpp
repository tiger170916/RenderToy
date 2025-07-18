#include "Pipeline.h"

Pipeline::Pipeline(std::string name, GUID guid)
	: m_guid(guid), m_name(name)
{

}

bool Pipeline::Initialize(GraphicsContext* graphicsContext)
{
	if (!graphicsContext)
	{
		return false;
	}

	m_commandQueue = std::unique_ptr<CommandQueue>(new CommandQueue(D3D12_COMMAND_QUEUE_FLAG_NONE, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_QUEUE_PRIORITY_NORMAL, graphicsContext->GetAdapterNodeMask()));
	if (!m_commandQueue->Initialize(graphicsContext->GetDevice()))
	{
		return false;
	}

	return true;
}

bool Pipeline::AddPass(PassBase* pPass)
{
	if (pPass == nullptr)
	{
		return false;
	}

	m_passes.push_back(pPass);

	return true;
}

bool Pipeline::AddPassDependency(PassBase* passAddDependencyTo, PassBase* dependency)
{
	if (passAddDependencyTo == nullptr || dependency == nullptr)
	{
		return false;
	}

	passAddDependencyTo->AddDependency(dependency);

	return true;
}

bool Pipeline::FindPass(PassBase* pass)
{
	for (auto& p : m_passes)
	{
		if (p == pass)
		{
			return true;
		}
	}

	return false;
}

Pipeline::~Pipeline()
{

}

