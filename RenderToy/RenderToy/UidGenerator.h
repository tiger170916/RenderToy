#pragma once

#include <Windows.h>
#include <memory>
#include "CriticalSection.h"

/// <summary>
/// (runtime) Universal id generator.
/// </summary>
class UidGenerator
{
private:
	static std::unique_ptr<UidGenerator> _singleton;

	uint32_t m_itr = 0;

	std::unique_ptr<CriticalSection> m_criticalSection = nullptr;

private:
	UidGenerator();

public:
	~UidGenerator();

	uint32_t GenerateUid();

	static UidGenerator* Get();
};