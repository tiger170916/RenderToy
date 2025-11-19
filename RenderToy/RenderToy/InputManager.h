#pragma once

#include "Includes.h"
#include "IControllable.h"

class InputManager
{
private:
	std::set<IControllable*> m_controlObjects;

	InputStruct m_inputState = {};

	uint64_t m_lastMouseMoveTime = 0;

	HWND m_hwnd = NULL;

public:
	static InputManager* Get();

	bool Initialize(HWND hwnd);

	void AddControlObject(IControllable* controlObject);

	void RemoveControlObject(IControllable* controlObject);

	~InputManager();


private:
	static void WindowProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, void* object);

	InputManager();

	void Update();

	void ClearActions();
};