#include "UidGenerator.h"

std::unique_ptr<UidGenerator> UidGenerator::_singleton = nullptr;

UidGenerator* UidGenerator::Get()
{
	if (!_singleton)
	{
		_singleton = std::unique_ptr<UidGenerator>(new UidGenerator());
	}

	return _singleton.get();
}

UidGenerator::UidGenerator()
{
	m_criticalSection = std::unique_ptr<CriticalSection>(new CriticalSection());
}

uint32_t UidGenerator::GenerateUid()
{
	m_criticalSection->EnterCriticalSection();

	uint32_t uid = ++m_itr;

	m_criticalSection->ExitCriticalSection();

	return uid;
}

UidGenerator::~UidGenerator()
{

}
