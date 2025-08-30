#pragma once

#include "Includes.h"

typedef void (*WndProcCallback)(HWND, UINT, WPARAM, LPARAM, void*);
#define WND_PROC_CALLBACK WndProcCallback

/// <summary>
/// WndProc class intercepts window messages and forwards them to registered WndProcs
/// </summary>
class WndProc
{
private:
	struct WndProcCallbackEntry
	{
		WndProcCallback funcAddr = nullptr;
		void* object = nullptr;
	};

private:
	static std::unique_ptr<WndProc> g_singleton;

	WNDPROC m_oldWndProc = nullptr;

	std::vector<WndProcCallbackEntry> m_registeredCallbacks;

	bool m_initialized = false;

public:
	static WndProc* Get();

	bool Initialize(HWND hwnd);

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void RegisterWndProc(WndProcCallback callback, void* object);

private:
	WndProc() {}
};