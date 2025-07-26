#include "CriticalSection.h"

CriticalSection::CriticalSection(DWORD timeout)
	: m_timeout(timeout)
{
	m_lock = CreateSemaphoreA(NULL, 1, 1, NULL);
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
	WaitForSingleObject(m_lock, INFINITE);
}

void CriticalSection::ExitCriticalSection()
{
	ReleaseSemaphore(m_lock, 1, nullptr);
}