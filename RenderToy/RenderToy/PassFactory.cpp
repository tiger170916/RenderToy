#include "PassFactory.h"
#include "EarlyZPass.h"
#include "GeometryPass.h"
#include "LightingPass.h"


std::map<GUID, std::unique_ptr<PassBase>, GuidComparator> PassFactory::_createdPasses;

bool PassFactory::CreatePass(PassType type, GUID passGuid, PassBase** ppPass)
{
	if (ppPass == nullptr)
	{
		return false;
	}

	*ppPass = GetPassTypeByGuid(passGuid);
	
	if (*ppPass)
	{
		return true;
	}

	PassBase* pass = nullptr;

	switch (type)
	{
	case PassType::EARLY_Z_PASS:
	{
		pass = new EarlyZPass(passGuid);
		break;
	}

	case PassType::GEOMETRY_PASS:
	{
		pass = new GeometryPass(passGuid);
		break;
	}
	case PassType::LIGHTING_PASS:
	{
		pass = new LightingPass(passGuid);
		break;
	}
	}

	_createdPasses[passGuid] = std::unique_ptr<PassBase>(pass);
	*ppPass = pass;
	return pass != nullptr;
}

PassBase* PassFactory::GetPassTypeByGuid(GUID guid)
{
	if (!_createdPasses.contains(guid))
	{
		return nullptr;
	}

	return _createdPasses[guid].get();
}