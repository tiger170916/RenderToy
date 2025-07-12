#include "GraphicsUtils.h"

void GraphicsUtils::ResourceBarrierTransition(ID3D12Resource* pResource, ID3D12GraphicsCommandList* pCommandList, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter)
{
    D3D12_RESOURCE_BARRIER transitionFromRenderTargetToPresent = CD3DX12_RESOURCE_BARRIER::Transition(
        pResource,
        stateBefore,
        stateAfter);

    pCommandList->ResourceBarrier(1, &transitionFromRenderTargetToPresent);
}