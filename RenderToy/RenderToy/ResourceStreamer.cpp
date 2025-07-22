#include "ResourceStreamer.h"

ResourceStreamer::ResourceStreamer()
{
	m_criticalSection = std::unique_ptr<CriticalSection>(new CriticalSection());
}

bool ResourceStreamer::StreamIn(StreamInterface* resource, UINT priority)
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
		m_p0Queue.push(resource);
	}
	else if (priority == 1)
	{
		m_p1Queue.push(resource);
	}
	else
	{
		m_p2Queue.push(resource);
	}

	m_criticalSection->ExitCriticalSection();

	return false;
}

bool ResourceStreamer::StreamOut(StreamInterface* resource)
{
	return false;
}

DWORD WINAPI ResourceStreamer::StreamInThreadProc(_In_ LPVOID lpParameter)
{
	return 1;
}

ResourceStreamer::~ResourceStreamer()
{

}