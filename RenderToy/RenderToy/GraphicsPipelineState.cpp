#include "GraphicsPipelineState.h"

GraphicsPipelineState::GraphicsPipelineState() {}

GraphicsPipelineState::~GraphicsPipelineState() {}

bool GraphicsPipelineState::Initialize(ID3D12Device* pDevice)
{
	if (!pDevice)
	{
		return false;
	}

	return true;
}