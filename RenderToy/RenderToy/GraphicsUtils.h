#pragma once

#include "Includes.h"

/// <summary>
/// Static class regarding graphics
/// </summary>
class GraphicsUtils
{
public:
	static void ResourceBarrierTransition(ID3D12Resource* pResource, ID3D12GraphicsCommandList* pCommandList, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter);
};