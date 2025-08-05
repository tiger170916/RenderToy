#pragma once

#include "../Includes.h"
#include "LightExtension.h"
#include "../Vectors.h"

class PointLight : public LightExtension
{
friend class LightFactory;

private:

protected:
	PointLight(float effectiveRange, FVector3 position, FVector3 intensity, uint32_t uid);

public:
	~PointLight();
};