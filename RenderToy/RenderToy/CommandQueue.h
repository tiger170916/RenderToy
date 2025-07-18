#pragma once
#include "Includes.h"
#include "CommandBuilder.h"

class CommandQueue
{
private:
	ComPtr<ID3D12CommandQueue> m_commandQueue = nullptr;

	ComPtr<ID3D12Fence> m_fence = nullptr;

	D3D12_COMMAND_QUEUE_FLAGS m_flags;

	D3D12_COMMAND_LIST_TYPE m_commandListType;

	HANDLE m_fenceEvent = 0;

	uint64_t m_fenceValue = 0;

	int m_priority = 0;

	int m_nodeMask = 0;

	bool m_initialized = false;

public:
	CommandQueue(D3D12_COMMAND_QUEUE_FLAGS flags, D3D12_COMMAND_LIST_TYPE commandListType, int priority, int nodeMask);

	bool Initialize(ID3D12Device* pDevice);

	bool DispatchCommands(CommandBuilder* cmdBuilder);

	void WaitForFence(ID3D12Fence* pFence, uint64_t fenceValue);

	void Signal(ID3D12Fence* pFence, uint64_t fenceValue);

	void SignalAndWait();

	inline ID3D12CommandQueue* GetCommandQueue() const { return m_commandQueue.Get(); }

	inline const D3D12_COMMAND_LIST_TYPE& GetCommandListType() const { return m_commandListType; }

	~CommandQueue();
};