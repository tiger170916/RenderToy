#include "ResourceStreamer.h"
#include "GraphicsUtils.h"

ResourceStreamer::ResourceStreamer()
{
	m_criticalSection = std::unique_ptr<CriticalSection>(new CriticalSection());
}

bool ResourceStreamer::StreamIn(std::shared_ptr<StreamInterface> resource, UINT priority)
{
	if (!resource)
	{
		return false;
	}

	if (resource->IsInMemory() && resource->HasCopiedToDefaultHeap())
	{
		return true;
	}

	m_criticalSection->EnterCriticalSection();

	if (priority == 0)
	{
		m_taskQueues[0].push(resource);
	}
	else if (priority == 1)
	{
		m_taskQueues[1].push(resource);
	}
	else
	{
		m_taskQueues[2].push(resource);
	}

	m_criticalSection->ExitCriticalSection();

	return true;
}

bool ResourceStreamer::StartStreaming(GraphicsContext* graphicsContext)
{
	if (m_running)
	{
		return true;
	}

	if (!graphicsContext)
	{
		return false;
	}

	m_graphicsContext = graphicsContext;

	for (int i = 0; i < 3; i++)
	{
		// Initialize command queues
		m_commandQueues[i] = std::unique_ptr<CommandQueue>(new CommandQueue(D3D12_COMMAND_QUEUE_FLAG_NONE, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_QUEUE_PRIORITY_HIGH, graphicsContext->GetAdapterNodeMask()));
		if (!m_commandQueues[i]->Initialize(graphicsContext->GetDevice()))
		{
			return false;
		}

		// Initialize command builders
		m_commandBuilders[i] = std::unique_ptr<CommandBuilder>(new CommandBuilder(D3D12_COMMAND_LIST_TYPE_DIRECT));
		if (!m_commandBuilders[i]->Initialize(graphicsContext->GetDevice()))
		{
			return false;
		}
	}

	m_running = true;

	// Create streaming in threads
	m_threadHandles[0] = CreateThread(NULL, 0, StreamInPriority0ThreadProc, this, NULL, nullptr);
	//m_threadHandles[1] = CreateThread(NULL, 0, StreamInPriority1ThreadProc, this, NULL, nullptr);
	//m_threadHandles[2] = CreateThread(NULL, 0, StreamInPriority2ThreadProc, this, NULL, nullptr);

	return true;
}

void ResourceStreamer::StopStreaming()
{
	if (!m_running)
	{
		return;
	}

	m_running = false;
	WaitForMultipleObjects(3, m_threadHandles, true, 2000);
	m_threadHandles[0] = NULL;
	m_threadHandles[1] = NULL;
	m_threadHandles[2] = NULL;
	m_threadHandles[3] = NULL;
}

bool ResourceStreamer::StreamOut(std::shared_ptr<StreamInterface>)
{
	return false;
}

void ResourceStreamer::StreamInInternal(const UINT& priority, const UINT& maxBatchSize, const bool& immediateCycleBack, const UINT& sleepTime, std::vector<std::shared_ptr<StreamInterface>>& streamingInObjects)
{
	m_criticalSection->EnterCriticalSection();

	size_t queueSize = m_taskQueues[priority].size();
	if (queueSize > 0)
	{
		for (size_t i = 0; i < min(maxBatchSize, queueSize); i++)
		{
			streamingInObjects.push_back(m_taskQueues[priority].front().lock());

			m_taskQueues[priority].pop();
		}
	}

	m_criticalSection->ExitCriticalSection();

	std::set<StreamInterface*> toCopy;
	for (auto& streamingInObj : streamingInObjects)
	{
		if (!streamingInObj)
		{
			continue;
		}

		if (streamingInObj->IsInMemory())
		{
			continue;
		}

		if (streamingInObj->StreamIn(m_graphicsContext))
		{
			toCopy.insert(streamingInObj.get());
		}
	}

	if (!toCopy.empty())
	{
		m_commandBuilders[priority]->TryReset();
		ID3D12GraphicsCommandList* commandList = m_commandBuilders[priority]->GetCommandList();

		for (auto& streamingInObj : toCopy)
		{
			if (!streamingInObj)
			{
				continue;
			}

			if (!streamingInObj->IsInMemory() || streamingInObj->HasCopiedToDefaultHeap())
			{
				continue;
			}

			streamingInObj->ScheduleForCopyToDefaultHeap(commandList);
		}

		m_commandBuilders[priority]->Close();
		m_commandQueues[priority]->DispatchCommands(m_commandBuilders[priority].get());
		m_commandQueues[priority]->SignalAndWait();

		for (auto& streamingInObj : toCopy)
		{
			if (!streamingInObj)
			{
				continue;
			}

			if (!streamingInObj)
			{
				continue;
			}

			streamingInObj->SetCopiedToDefaultHeap(true);
		}
	}

	streamingInObjects.clear();

	if (!immediateCycleBack)
	{
		Sleep(sleepTime);
	}
	else if (m_taskQueues[priority].empty())
	{
		Sleep(sleepTime);
	}
}

DWORD WINAPI ResourceStreamer::StreamInPriority0ThreadProc(_In_ LPVOID lpParameter)
{
	ResourceStreamer* streamer = (ResourceStreamer*)lpParameter;
	if (!streamer)
	{
		return 0;
	}
	
	
	std::vector<std::shared_ptr<StreamInterface>> streamingInObjects;
	while (streamer->m_running)
	{
		streamer->StreamInInternal(0, 100, true, 1, streamingInObjects);
	}

	return 1;
}

DWORD WINAPI ResourceStreamer::StreamInPriority1ThreadProc(_In_ LPVOID lpParameter)
{
	ResourceStreamer* streamer = (ResourceStreamer*)lpParameter;
	if (!streamer)
	{
		return 0;
	}


	std::vector<std::shared_ptr<StreamInterface>> streamingInObjects;
	while (streamer->m_running)
	{
		streamer->StreamInInternal(1, 50, false, 10, streamingInObjects);
	}

	return 1;
}

DWORD WINAPI ResourceStreamer::StreamInPriority2ThreadProc(_In_ LPVOID lpParameter)
{
	ResourceStreamer* streamer = (ResourceStreamer*)lpParameter;
	if (!streamer)
	{
		return 0;
	}


	std::vector<std::shared_ptr<StreamInterface>> streamingInObjects;
	while (streamer->m_running)
	{
		streamer->StreamInInternal(2, 10, false, 50, streamingInObjects);
	}

	return 1;
}

ResourceStreamer::~ResourceStreamer()
{

}