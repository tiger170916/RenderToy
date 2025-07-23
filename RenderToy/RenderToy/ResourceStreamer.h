#pragma once

#include "Includes.h"
#include "StreamInterface.h"
#include "CriticalSection.h"
#include "CommandBuilder.h"
#include "CommandQueue.h"
#include "D3DFence.h"
#include "GraphicsContext.h"

class ResourceStreamer
{
private:
	GraphicsContext* m_graphicsContext = nullptr;

	std::unique_ptr<CommandQueue> m_commandQueues[4] = { nullptr };

	std::unique_ptr<CommandBuilder> m_commandBuilders[4] = { nullptr };

	std::queue<std::weak_ptr<StreamInterface>> m_taskQueues[4] = { };

	HANDLE m_threadHandles[4] = { NULL };

	std::unique_ptr<CriticalSection> m_criticalSection;

	bool m_running = false;

public:
	ResourceStreamer();

	bool StartStreaming(GraphicsContext* graphicsContext);

	void StopStreaming();

	~ResourceStreamer();

	bool StreamIn(std::shared_ptr<StreamInterface>, UINT priority);

	bool StreamOut(std::shared_ptr<StreamInterface> resource);

private:
	static DWORD WINAPI StreamInPriority0ThreadProc(_In_ LPVOID lpParameter);

	static DWORD WINAPI StreamInPriority1ThreadProc(_In_ LPVOID lpParameter);

	static DWORD WINAPI StreamInPriority2ThreadProc(_In_ LPVOID lpParameter);

private:
	void StreamInInternal(const UINT& priority, const UINT& maxBatchSize, const bool& immediateCycleBack, const UINT& sleepTime, std::vector<std::shared_ptr<StreamInterface>>& streamingInObjects);
};