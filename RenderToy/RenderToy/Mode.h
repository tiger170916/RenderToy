#pragma once
#include "IControllable.h"

class World2;

/// <summary>
/// Mode class manages the world or game logic
/// </summary>
class Mode : public IControllable
{
private:
	World2* m_activeWorld = nullptr;

public:
	Mode();

	inline void SetWorld(World2* world) { m_activeWorld = world; }

	void ProcessInput(InputStruct inputStruct) override;
};