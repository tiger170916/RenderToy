#pragma once

#include "Includes.h"
#include "IControllable.h"

class InputManager
{
private:
	std::unique_ptr<DirectX::Keyboard> m_keyboard;

	std::unique_ptr<DirectX::Mouse> m_mouse;

	IControllable* m_avtiveControlObject = nullptr;	// TODO: extend to be able to control multiple objects

public:
	bool Initialize(HWND hwnd);

	void Update(float delta);

	void SetControlObject(IControllable* controlObject) { m_avtiveControlObject = controlObject; }

	~InputManager();


private:
	static void WindowProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, void* object);
};