#pragma once

#include "Includes.h"

/// <summary>
/// A commandAllocator and commandList pair wrapper class
/// </summary>
class CommandBuilder
{
friend class CommandQueue;
private:

	ComPtr<ID3D12CommandAllocator> m_commandAllocator = nullptr;

	ComPtr<ID3D12GraphicsCommandList> m_commandList = nullptr;

	D3D12_COMMAND_LIST_TYPE m_commandListType;

	bool m_initialized = false;

	bool m_hasRecordedCommand = false;

	bool m_recording = false;

protected:
	inline void ClearRecordedCommand() { m_hasRecordedCommand = false; }

public:
	CommandBuilder(D3D12_COMMAND_LIST_TYPE cmdListType);

	bool Initialize(ID3D12Device* pDevice);

	/// <summary>
	/// Try reset command allocator and command list, if not in recording state
	/// </summary>
	void TryReset();

	void Close();

	inline ID3D12GraphicsCommandList* GetCommandList() { return m_commandList.Get(); }

	inline const D3D12_COMMAND_LIST_TYPE& GetCommandListType() const { return m_commandListType; }

	inline const bool HasRecordedCommands() const { return m_hasRecordedCommand; };

	~CommandBuilder();
};