#pragma once

#include "Includes.h"
#include "Vectors.h"

class DescriptorHeapManager;
/// <summary>
/// Static class regarding graphics
/// </summary>
class GraphicsUtils
{
public:
	static void ResourceBarrierTransition(
		ID3D12Resource* pResource, 
		ID3D12GraphicsCommandList* pCommandList, 
		D3D12_RESOURCE_STATES stateBefore,
		D3D12_RESOURCE_STATES stateAfter);

	static bool CreateDepthStencilResource(
		ID3D12Device* pDevice, 
		DescriptorHeapManager* pDescHeapMgr,
		UINT width, 
		UINT height, 
		DXGI_FORMAT pixelFormat, 
		D3D12_RESOURCE_FLAGS extraFlags,
		float depthClearValue, 
		UINT8 stencilClearValue, 
		ID3D12Resource** ppResource,
		UINT64& outDsvId);

	static bool CreateRenderTargetResource(
		ID3D12Device* pDevice,
		DescriptorHeapManager* pDescHeapMgr,
		UINT width,
		UINT height,
		DXGI_FORMAT pixelFormat,
		D3D12_RESOURCE_FLAGS extraFlags,
		const float clearColor[4],
		ID3D12Resource** ppResource,
		UINT64& outRtvId);

	static XMMATRIX ViewMatrixFromPositionRotation(const FVector3& position, const FRotator& rotator, FVector3& outForwardDir);
};