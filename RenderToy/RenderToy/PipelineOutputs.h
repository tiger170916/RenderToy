#pragma once

#include "Includes.h"

enum PipelinePassOutputType
{
	NONE = 0,
	EARLY_Z_PASS_DEPTH_BUFFER = 1,
};

struct PipelineOutput
{
	ID3D12Resource* pResource;
	UINT64 SrvId;
};

struct PipelineOutputsStruct
{
	std::map<PipelinePassOutputType, PipelineOutput> Outputs;
};

class PipelineResourceStates
{
private:
	std::map<ID3D12Resource*, D3D12_RESOURCE_STATES> m_resourceStates;

public:
	PipelineResourceStates();

	~PipelineResourceStates();

	bool ChangeState(ID3D12Resource* resource, D3D12_RESOURCE_STATES newState);

	bool GetState(ID3D12Resource* resource, D3D12_RESOURCE_STATES& outState);

	void AddResourceState(ID3D12Resource* resource, D3D12_RESOURCE_STATES initState);
};