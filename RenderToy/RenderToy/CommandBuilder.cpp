#include "CommandBuilder.h"

CommandBuilder::CommandBuilder(D3D12_COMMAND_LIST_TYPE cmdListType)
	: m_commandListType(cmdListType)
{

}

bool CommandBuilder::Initialize(ID3D12Device* pDevice)
{
	if (m_initialized)
	{
		return true;
	}

	if (!pDevice)
	{
		return false;
	}

	if (FAILED(pDevice->CreateCommandAllocator(m_commandListType, IID_PPV_ARGS(m_commandAllocator.GetAddressOf()))))
	{
		return false;
	}

	if (FAILED(pDevice->CreateCommandList(0, m_commandListType, m_commandAllocator.Get(), NULL, IID_PPV_ARGS(m_commandList.GetAddressOf()))))
	{
		return false;
	}

	m_commandList->Close();

	m_initialized = true;
	return true;
}

void CommandBuilder::Reset()
{
	if (m_commandAllocator == nullptr ||
		m_commandList == nullptr)
	{
		return;
	}

	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator.Get(), nullptr);
}

void CommandBuilder::Close()
{
	if (m_commandList == nullptr)
	{
		return;
	}

	m_commandList->Close();
}

CommandBuilder::~CommandBuilder()
{
	if (m_commandList)
	{
		m_commandList.Reset();
	}

	if (m_commandAllocator)
	{
		m_commandAllocator.Reset();
	}
}