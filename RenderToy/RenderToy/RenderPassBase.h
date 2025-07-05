#pragma once

#include "Includes.h"
#include "World.h"

class RenderPassBase
{
public:
	virtual void Frame(std::shared_ptr<World> world) = 0;
};