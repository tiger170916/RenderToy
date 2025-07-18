#include "D3DFence.h"

D3DFence::D3DFence()
{

}

D3DFence::~D3DFence()
{

}

bool D3DFence::Initialize(ID3D12Device* pDevice)
{
	if (!pDevice)
	{
		return false;
	}

	m_fenceValue = 0;

	m_initialize = SUCCEEDED(pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.GetAddressOf())));
	return m_initialize;
}

void D3DFence::Signal()
{
	if (m_fence)
	{
		m_fenceValue++;
		m_fence->Signal(m_fenceValue);
	}
}

void D3DFence::CommandQueueSignal(ID3D12CommandQueue* commandQueue)
{
	if (!commandQueue)
	{
		return;
	}

	m_fenceValue += 1;
	commandQueue->Signal(m_fence.Get(), m_fenceValue);
}