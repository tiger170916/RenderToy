#pragma once
#include <Windows.h>
class GuidComparator
{
public:
    bool operator()(const GUID& left, const GUID& right) const;
};