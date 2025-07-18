#pragma once
#include "Includes.h"

class GuidComparator
{
public:
    bool operator()(const GUID& left, const GUID& right) const;
};