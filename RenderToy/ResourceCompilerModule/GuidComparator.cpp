#include "GuidComparator.h"
bool GuidComparator::operator()(const GUID& left, const GUID& right) const
{
    if (left.Data1 != right.Data1)
        return left.Data1 < right.Data1;
    if (left.Data2 != right.Data2)
        return left.Data2 < right.Data2;
    if (left.Data3 != right.Data3)
        return left.Data3 < right.Data3;

    return memcmp(left.Data4, right.Data4, sizeof(left.Data4)) < 0;
}