#pragma once

#include "Includes.h"
#include "GraphicsContext.h"

class StreamInterface
{
protected:
	bool m_inUploadHeapMemory = false;

	bool m_copiedToDefaultHeap = false;

public:
	// Interfaces
	
	/// <summary>
	/// Perform streaming in
	/// </summary>
	virtual bool StreamIn(GraphicsContext* graphicsContext) = 0;

	/// <summary>
	/// Perform streaming out
	/// </summary>
	virtual bool StreamOut() = 0;

	/// <summary>
	/// Schedule a command to copy the buffer from upload heap to default heap, if needed.
	/// </summary>
	virtual bool ScheduleForCopyToDefaultHeap(ID3D12GraphicsCommandList* cmdList) = 0;

public:
	inline const bool IsInMemory() const { return m_inUploadHeapMemory; }

	inline const bool HasCopiedToDefaultHeap() const { return m_copiedToDefaultHeap; }
};