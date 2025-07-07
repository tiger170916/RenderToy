#pragma once

#include "Includes.h"

class DescriptorHeapManager
{
private:
	const UINT m_defaultSrvUavDescriptorHeapSize = 128;

	const UINT m_defaultRtvDescriptorHeapSize = 64;

	const UINT m_defaultDsvDescriptorHeapSize = 64;

	UINT m_srvDescriptorSize = 0;

	UINT m_dsvDescriptorSize = 0;

	UINT m_rtvDescriptorSize = 0;

	std::vector<ComPtr<ID3D12DescriptorHeap>> m_srvUavDescriptorHeaps;

	ComPtr<ID3D12DescriptorHeap> m_rtvDescriptorHeap = nullptr;

	ComPtr<ID3D12DescriptorHeap> m_dsvDescriptorHeap = nullptr;

public:
	DescriptorHeapManager();

	bool Initialize(ID3D12Device* pDevice);

	~DescriptorHeapManager();
};