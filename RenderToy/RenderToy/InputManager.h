#pragma once

#include "Includes.h"
#include "IControllable.h"

class InputManager
{
private:
	std::unique_ptr<DirectX::Keyboard> m_keyboard;

	std::unique_ptr<DirectX::Mouse> m_mouse;

	std::set<IControllable*> m_controlObjects;


public:
	bool Initialize(HWND hwnd);

	void Update(float delta);

	void AddControlObject(IControllable* controlObject);

	void RemoveControlObject(IControllable* controlObject);

	~InputManager();


private:
	static void WindowProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, void* object);
};