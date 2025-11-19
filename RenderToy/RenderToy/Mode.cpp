#include "Mode.h"
#include "World2.h"

Mode::Mode()
{
}

void Mode::ProcessInput(InputStruct inputStruct)
{
	if (m_activeWorld == nullptr)
	{
		return;
	}

	if (inputStruct.SwitchCharacter)
	{
		m_activeWorld->SwitchPlayableObject();
	}

	if (inputStruct.SwitchCamera)
	{
		m_activeWorld->SwitchCamera();
	}
}