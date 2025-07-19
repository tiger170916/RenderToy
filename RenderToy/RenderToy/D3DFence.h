#pragma once

#include "Includes.h"

class D3DFence
{
private:
	ComPtr<ID3D12Fence> m_fence = nullptr;

	uint64_t m_fenceValue = 0;

	bool m_initialize = false;

public:
	D3DFence();

	~D3DFence();

	bool Initialize(ID3D12Device* pDevice);

	void Signal();

	void CommandQueueSignal(ID3D12CommandQueue* commandQueue);

	void CommandQueueSignalAndSetEvent(ID3D12CommandQueue* commandQueue, HANDLE hEvent);

	inline ID3D12Fence* GetFence() { return m_fence.Get(); }

	inline uint64_t GetCurrentFenceValue() const { return m_fenceValue; }
};