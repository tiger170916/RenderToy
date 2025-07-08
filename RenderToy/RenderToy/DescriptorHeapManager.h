#pragma once

#include "Includes.h"

/// <summary>
/// A descriptor heap manager, which keeps view creation and binding abstract
/// </summary>
class DescriptorHeapManager
{
private:
	const UINT m_defaultCbvSrvUavDescriptorHeapSize = 64;

	const UINT m_defaultRtvDescriptorHeapSize = 64;

	const UINT m_defaultDsvDescriptorHeapSize = 64;

	const UINT m_defaultSamplerDescriptorHeapSize = 64;

	const UINT m_defaultRingBufferSize = 1024;

	UINT m_rtvDescriptorSize = 0;

	UINT m_dsvDescriptorSize = 0;

	UINT m_cbvSrvUavDescriptorSize = 0;

	UINT m_samplerDescriptorSize = 0;

	UINT m_currentRtvCount = 0;

	UINT m_currentDsvCount = 0;

	UINT m_currentSamplerCount = 0;

	ComPtr<ID3D12DescriptorHeap> m_rtvDescriptorHeap = nullptr;

	ComPtr<ID3D12DescriptorHeap> m_dsvDescriptorHeap = nullptr;

	ComPtr<ID3D12DescriptorHeap> m_samplerDescriptorHeap = nullptr;

	ComPtr<ID3D12DescriptorHeap> m_cbvSrvUavRingBuffer = nullptr;

	std::vector<ComPtr<ID3D12DescriptorHeap>> m_cbvSrvUavDescriptorHeaps;

	std::vector<std::vector<int>> m_cbvSrvUavDescriptorPositions;

	std::vector<uint64_t> m_cbvSrvUavRingBufferIds;

	UINT m_ringBufferItr = 0;

	ID3D12Device* m_pDevice = nullptr;

	UINT m_adapterNodeMask = 0;

	bool m_initialized = false;

public:
	DescriptorHeapManager(ID3D12Device* pDevice, UINT adapterNodeMask);

	bool Initialize();

	/// <summary>
	/// Create an rtv,
	/// </summary>
	/// <returns>id</returns>
	uint64_t CreateRenderTargetView(ID3D12Resource* pResource, const D3D12_RENDER_TARGET_VIEW_DESC* pDesc);

	/// <summary>
	/// Create a dsv
	/// </summary>
	/// <returns>id</returns>
	uint64_t CreateDepthStencilView(ID3D12Resource* pResource, const D3D12_DEPTH_STENCIL_VIEW_DESC* pDesc);

	/// <summary>
	/// Create sampler
	/// </summary>
	/// <returns>id</returns>
	uint64_t CreateSampler(const D3D12_SAMPLER_DESC* pDesc);

	/// <summary>
	/// Create constant buffer view
	/// </summary>
	/// <returns>id</returns>
	uint64_t CreateConstantBufferView(const D3D12_CONSTANT_BUFFER_VIEW_DESC* pDesc);

	/// <summary>
	/// Create shader resource view
	/// </summary>
	/// <returns>id</returns>
	uint64_t CreateShaderResourceView(ID3D12Resource* pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc);

	/// <summary>
	/// Create unordered access view
	/// </summary>
	/// <returns>id</returns>
	uint64_t CreateUnorderedAccessView(ID3D12Resource* pResource, ID3D12Resource* pCounterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* pDesc);

	/// <summary>
	/// Bind the view of given id to current pipeline
	/// </summary>
	bool BindCbvSrvUavToPipeline(uint64_t viewId, D3D12_GPU_DESCRIPTOR_HANDLE& outGpuDescriptorHandle);

	/// <summary>
	/// Get render target view handle
	/// </summary>
	/// <param name="viewId"></param>
	bool GetRenderTargetViewCpuHandle(uint64_t viewId, D3D12_CPU_DESCRIPTOR_HANDLE& outCpuDescriptorHandle);

	/// <summary>
	/// Get depth stencil view handle
	/// </summary>
	/// <param name="viewId"></param>
	bool GetDepthStencilViewCpuHandle(uint64_t viewId, D3D12_CPU_DESCRIPTOR_HANDLE& outCpuDescriptorHandle);

	/// <summary>
	/// Get sampler handle
	/// </summary>
	/// <param name="viewId"></param>
	bool GetSampler(uint64_t viewId, D3D12_GPU_DESCRIPTOR_HANDLE& outGpuDescriptorHandle);

	/// <summary>
	/// Get ring buffer cbv/srv/uav shader visible descriptor heap
	/// </summary>
	inline ID3D12DescriptorHeap* GetCbvSrvUavShaderVisibleRingBufferHeap() const { return m_cbvSrvUavRingBuffer.Get(); }

	/// <summary>
	/// Get sampler shader visible descriptor heap
	/// </summary>
	inline ID3D12DescriptorHeap* GetSamplerShaderVisibleDescriptorHeap() const { return m_samplerDescriptorHeap.Get(); }

	/// <summary>
	/// Get render target view descriptor heap
	/// </summary>
	inline ID3D12DescriptorHeap* GetRenderTargetViewDescriptorHeap() const { return m_rtvDescriptorHeap.Get(); }

	/// <summary>
	/// Get depth stencil view descriptor heap
	/// </summary>
	inline ID3D12DescriptorHeap* GetDepthStencilViewDescriptorHeap() const { return m_dsvDescriptorHeap.Get(); }

	~DescriptorHeapManager();


private:
	// Add a new cbv/srv/uav non shader visible descriptor heap when needed.
	bool AddNewCbvSrvUavDescriptorHeap();

	// Get the id of cbv/srv/uav of given non shader visible descriptor heap.
	uint64_t GetIdOfCbvSrvUav(uint64_t arrayIdx, uint64_t elementIdx);

	// Get the array idx and element idx given viewId
	void GetArrayIdxAndElementIdxByViewId(uint64_t viewId, uint64_t& arrayIdx, uint64_t& elementIdx);
};