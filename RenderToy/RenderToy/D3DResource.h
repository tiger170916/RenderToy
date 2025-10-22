#pragma once

#include "Includes.h"
#include "GraphicsContext.h"

/// <summary>
/// A d3d12Resource wrapper class, represents a resource may or may not be copied to default heap
/// </summary>
class D3DResource
{
private:
	bool m_needCopyToDefaultHeap = false;

	bool m_copiedToDefaultHeap = false;

	ComPtr<ID3D12Resource> m_defaultHeapResource = nullptr;

	ComPtr<ID3D12Resource> m_uploadHeapResource = nullptr;

	D3D12_RESOURCE_STATES m_defaultBufferCurrentState = D3D12_RESOURCE_STATE_COMMON;

	D3D12_RESOURCE_STATES m_uploadBufferCurrentState = D3D12_RESOURCE_STATE_COMMON;

	bool m_isTexture = false;

	UINT m_width = 0;

	UINT m_height = 0;

	UINT m_depth = 0;

	DXGI_FORMAT m_dxgiFormat = DXGI_FORMAT_UNKNOWN;

	UINT m_stride = 0;

	bool m_initialized = false;
 
public:
	D3DResource(bool needCopyToDefaultHeap);

	bool Initialize(GraphicsContext* graphicsContext, D3D12_RESOURCE_DESC* pResourceDesc, void* data, UINT dataSize, UINT stride);

	inline ID3D12Resource* GetDefaultResource() const { return m_defaultHeapResource.Get(); }

	inline ID3D12Resource* GetUploadResource() const { return m_uploadHeapResource.Get(); }

	bool UpdateUploadBuffer(void* data, UINT size);

	bool CopyToDefaultHeap(ID3D12GraphicsCommandList* commandList);

	bool CleanUploadResource();

	~D3DResource();
};