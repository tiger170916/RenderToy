#pragma once

#include "Includes.h"
#include "StreamInterface.h"
#include "CriticalSection.h"
#include "CommandBuilder.h"
#include "CommandQueue.h"

class ResourceStreamer
{
private:
	std::queue<StreamInterface*> m_p0Queue;

	std::queue<StreamInterface*> m_p1Queue;

	std::queue<StreamInterface*> m_p2Queue;

	std::queue<StreamInterface*> m_streamOutQueue;

	std::unique_ptr<CriticalSection> m_criticalSection;

public:
	ResourceStreamer();

	~ResourceStreamer();

	bool StreamIn(StreamInterface* resource, UINT priority);

	bool StreamOut(StreamInterface* resource);

private:
	static DWORD WINAPI StreamInThreadProc(_In_ LPVOID lpParameter);
};