#pragma once

#include "Includes.h"

/// <summary>
/// A commandAllocator and commandList pair wrapper class
/// </summary>
class CommandBuilder
{
private:

	ComPtr<ID3D12CommandAllocator> m_commandAllocator = nullptr;

	ComPtr<ID3D12GraphicsCommandList> m_commandList = nullptr;

	D3D12_COMMAND_LIST_TYPE m_commandListType;

	bool m_initialized = false;

public:
	CommandBuilder(D3D12_COMMAND_LIST_TYPE cmdListType);

	bool Initialize(ID3D12Device* pDevice);

	/// <summary>
	/// Reset the cmd builder to recording state0.
	/// </summary>
	void Reset();

	void Close();

	inline ID3D12GraphicsCommandList* GetCommandList() { return m_commandList.Get(); }

	inline const D3D12_COMMAND_LIST_TYPE& GetCommandListType() const { return m_commandListType; }

	~CommandBuilder();
};