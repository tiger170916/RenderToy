#include "CommandQueue.h"

CommandQueue::CommandQueue(D3D12_COMMAND_QUEUE_FLAGS flags, D3D12_COMMAND_LIST_TYPE commandListType, int priority, int nodeMask)
	: m_flags(flags), m_commandListType(m_commandListType), m_priority(priority), m_nodeMask(nodeMask)
{
}

bool CommandQueue::Initialize(ID3D12Device* pDevice)
{
	if (m_initialized)
	{
		return true;
	}

	if (!pDevice)
	{
		return false;
	}

	D3D12_COMMAND_QUEUE_DESC cqDesc = {}; // we will be using all the default values
	cqDesc.Type = m_commandListType;
	cqDesc.Priority = m_priority;
	cqDesc.Flags = m_flags;
	cqDesc.NodeMask = m_nodeMask;

	if (FAILED(pDevice->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(m_commandQueue.GetAddressOf()))))
	{
		return false;
	}

	if (FAILED(pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.GetAddressOf()))))
	{
		return false;
	}

	m_fenceEvent = CreateEventA(nullptr, FALSE, FALSE, nullptr);

	m_initialized = true;

	return true;
}

bool CommandQueue::DispatchCommands(CommandBuilder* cmdBuilder)
{
	if (!cmdBuilder || cmdBuilder->GetCommandListType() != m_commandListType)
	{
		return false;
	}

	ID3D12GraphicsCommandList* cmdList = cmdBuilder->GetCommandList();
	if (!cmdList)
	{
		return false;
	}

	ID3D12CommandList* cmdLists[] = { cmdList };
	m_commandQueue->ExecuteCommandLists(1, cmdLists);

	return true;
}

void CommandQueue::SignalAndWait()
{
	m_commandQueue->Signal(m_fence.Get(), m_fenceValue);

	if (SUCCEEDED(m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent)))
	{
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}
}

void CommandQueue::WaitForFence(ID3D12Fence* pFence, uint64_t fenceValue)
{
	if (!pFence)
	{
		return;
	}

	m_commandQueue->Wait(pFence, fenceValue);
}

void CommandQueue::Signal(ID3D12Fence* pFence, uint64_t fenceValue)
{
	if (!pFence)
	{
		return;
	}

	m_commandQueue->Signal(pFence, fenceValue);
}

CommandQueue::~CommandQueue()
{
	if (m_fence)
	{
		m_fence.Reset();
	}

	if (m_commandQueue)
	{
		m_commandQueue.Reset();
	}
}