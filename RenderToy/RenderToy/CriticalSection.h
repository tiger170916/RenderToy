#pragma once

#include "Includes.h"

class CriticalSection
{
private:
	HANDLE m_lock = NULL;

	bool m_timeout;

public:
	CriticalSection(DWORD timeout = INFINITE);

	~CriticalSection();

	void EnterCriticalSection();

	void ExitCriticalSection();
};