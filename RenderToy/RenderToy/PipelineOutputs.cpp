#include "PipelineOutputs.h"

PipelineResourceStates::PipelineResourceStates() {}

PipelineResourceStates::~PipelineResourceStates() {}

bool PipelineResourceStates::ChangeState(ID3D12Resource* resource, D3D12_RESOURCE_STATES newState)
{
	if (!m_resourceStates.contains(resource))
	{
		return false;
	}

	m_resourceStates[resource] = newState;
	return false;
}

bool PipelineResourceStates::GetState(ID3D12Resource* resource, D3D12_RESOURCE_STATES& outState)
{
	if (!m_resourceStates.contains(resource))
	{
		return false;
	}

	outState = m_resourceStates[resource];
	return true;
}

void PipelineResourceStates::AddResourceState(ID3D12Resource* resource, D3D12_RESOURCE_STATES initState)
{
	if (m_resourceStates.contains(resource))
	{
		return;
	}

	m_resourceStates[resource] = initState;
}