#pragma once

#include "Includes.h"
#include "PassDefines.h"
#include "PassBase.h"
#include "GuidComparator.h"

class PassFactory
{
private:
	static std::map<GUID, std::unique_ptr<PassBase>, GuidComparator> _createdPasses;

public:
	static bool CreatePass(PassType type, GUID passGuid, PassBase** pPaseBase);

	static PassBase* GetPassTypeByGuid(GUID guid);
};