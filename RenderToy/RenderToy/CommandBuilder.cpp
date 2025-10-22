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

void CommandBuilder::TryReset()
{
	if (m_commandAllocator == nullptr ||
		m_commandList == nullptr)
	{
		return;
	}

	if (m_recording)
	{
		return;
	}

	m_recording = true;
	m_hasRecordedCommand = true;

	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator.Get(), nullptr);
}

void CommandBuilder::Close()
{
	if (m_commandList == nullptr)
	{
		return;
	}

	if (m_recording)
	{
		m_commandList->Close();
		m_recording = false;
	}
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