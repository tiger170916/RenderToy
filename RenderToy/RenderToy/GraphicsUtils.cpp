#include "GraphicsUtils.h"
#include "DescriptorHeapManager.h"

void GraphicsUtils::ResourceBarrierTransition(
	ID3D12Resource* pResource, 
	ID3D12GraphicsCommandList* pCommandList, 
	D3D12_RESOURCE_STATES stateBefore, 
	D3D12_RESOURCE_STATES stateAfter)
{
	if (stateBefore == stateAfter)
	{
		return;
	}

    D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        pResource,
        stateBefore,
        stateAfter);

    pCommandList->ResourceBarrier(1, &barrier);
}

bool GraphicsUtils::CreateDepthStencilResource(
	ID3D12Device* pDevice,
	DescriptorHeapManager* pDescHeapMgr,
	UINT width,
	UINT height,
	DXGI_FORMAT pixelFormat,
	D3D12_RESOURCE_FLAGS extraFlags,
	float depthClearValue,
	UINT8 stencilClearValue,
	ID3D12Resource** ppResource,
	UINT64& outDsvId)
{
	if (!pDevice || !pDescHeapMgr || !ppResource)
	{
		return false;
	}

	// Create the depth/stencil buffer and view.
	D3D12_RESOURCE_DESC depthStencilResourceDesc = {};
	depthStencilResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilResourceDesc.Alignment = 0;
	depthStencilResourceDesc.Width = width;
	depthStencilResourceDesc.Height = height;
	depthStencilResourceDesc.DepthOrArraySize = 1;
	depthStencilResourceDesc.MipLevels = 1;
	depthStencilResourceDesc.SampleDesc.Count = 1;
	depthStencilResourceDesc.SampleDesc.Quality = 0;
	depthStencilResourceDesc.Format = pixelFormat;
	depthStencilResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | extraFlags;

	D3D12_CLEAR_VALUE optClear = {};
	optClear.Format = pixelFormat;
	optClear.DepthStencil.Depth = depthClearValue;
	optClear.DepthStencil.Stencil = stencilClearValue;

	D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	if FAILED(pDevice->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&depthStencilResourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&optClear,
		IID_PPV_ARGS(ppResource)))
	{
		return false;
	}

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = pixelFormat;
	dsvDesc.Texture2D.MipSlice = 0;

	outDsvId = pDescHeapMgr->CreateDepthStencilView(*ppResource, &dsvDesc);

	return outDsvId != UINT64_MAX;
}

bool GraphicsUtils::CreateRenderTargetResource(
	ID3D12Device* pDevice,
	DescriptorHeapManager* pDescHeapMgr,
	UINT width,
	UINT height,
	DXGI_FORMAT pixelFormat,
	D3D12_RESOURCE_FLAGS extraFlags,
	const float clearColor[4],
	ID3D12Resource** ppResource,
	UINT64& outRtvId)
{
	if (!pDevice || !pDescHeapMgr || !ppResource)
	{
		return false;
	}

	// Create the depth/stencil buffer and view.
	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = width;
	resourceDesc.Height = height;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Format = pixelFormat;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | extraFlags;

	D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	D3D12_CLEAR_VALUE optClear = {};
	optClear.Format = pixelFormat;
	optClear.Color[0] = clearColor[0];
	optClear.Color[1] = clearColor[1];
	optClear.Color[2] = clearColor[2];
	optClear.Color[3] = clearColor[3];

	if (FAILED(pDevice->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_PRESENT,
		&optClear,
		IID_PPV_ARGS(ppResource))))
	{
		return false;
	}

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = pixelFormat;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;

	outRtvId = pDescHeapMgr->CreateRenderTargetView(*ppResource, &rtvDesc);

	return outRtvId != UINT64_MAX;
}

XMMATRIX GraphicsUtils::ViewMatrixFromPositionRotation(const FVector3& position, const FRotator& rotator) 
{
	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(rotator.Pitch, rotator.Yaw, rotator.Roll);

	XMVECTOR forwardDirXM = XMVector3Transform(XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f), rotationMatrix);
	//XMVECTOR rightDirXM = XMVector3Transform(XMVectorSet(m_basisRight.X, m_basisRight.Y, m_basisRight.Z, 1.0f), rotationMatrix);
	XMVECTOR upDirXM = XMVector3Transform(XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), rotationMatrix);


	XMVector3Normalize(forwardDirXM);
	//XMVector3Normalize(rightDirXM);
	XMVector3Normalize(upDirXM);

	FVector3 forwardDirection = FVector3(forwardDirXM.m128_f32[0], forwardDirXM.m128_f32[1], forwardDirXM.m128_f32[2]);
	//FVector3 rightDirection = FVector3(rightDirXM.m128_f32[0], rightDirXM.m128_f32[1], rightDirXM.m128_f32[2]);
	FVector3 upDirection = FVector3(upDirXM.m128_f32[0], upDirXM.m128_f32[1], upDirXM.m128_f32[2]);

	FVector3 focusPos = position + forwardDirection;


	return XMMatrixLookAtLH(
		XMVectorSet(position.X, position.Y, position.Z, 1.0f),
		XMVectorSet(focusPos.X, focusPos.Y, focusPos.Z, 1.0f),
		XMVectorSet(upDirection.X, upDirection.Y, upDirection.Z, 1.0f));
}