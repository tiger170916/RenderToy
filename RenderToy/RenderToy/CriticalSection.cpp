#include "CriticalSection.h"

CriticalSection::CriticalSection(DWORD timeout)
	: m_timeout(timeout)
{
	m_lock = CreateMutexA(NULL, true, NULL);
}

CriticalSection::~CriticalSection()
{
	if (m_lock)
	{
		CloseHandle(m_lock);
		m_lock = NULL;
	}
}

void CriticalSection::EnterCriticalSection()
{
	WaitForSingleObject(m_lock, m_timeout);
}

void CriticalSection::ExitCriticalSection()
{
	ReleaseMutex(m_lock);
}