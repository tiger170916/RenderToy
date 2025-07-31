#pragma once

#include "../Includes.h"
#include "LightExtension.h"
#include "../Vectors.h"

class PointLight : public LightExtension
{
friend class LightFactory;

private:

protected:
	PointLight(float effectiveRange, FVector3 position, FVector3 intensity);

public:
	~PointLight();
};