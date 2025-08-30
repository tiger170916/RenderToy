#include "WndProc.h"

std::unique_ptr<WndProc> WndProc::g_singleton = nullptr;

WndProc* WndProc::Get()
{
	if (g_singleton == nullptr)
	{
		g_singleton = std::unique_ptr<WndProc>(new WndProc());
	}

	return g_singleton.get();
}

bool WndProc::Initialize(HWND hwnd)
{
	m_oldWndProc = (WNDPROC)GetWindowLongPtr(hwnd, GWLP_WNDPROC);

	SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)WndProc::WindowProc);

	return true;
}

void WndProc::RegisterWndProc(WndProcCallback callback, void* object)
{
	if (callback == nullptr)
	{
		return;
	}

	for (auto& callbackStruct : m_registeredCallbacks)
	{
		if (callbackStruct.funcAddr == callback && callbackStruct.object == object)
		{
			return;
		}
	}

	WndProcCallbackEntry entry{ .funcAddr = callback, .object = object };
	m_registeredCallbacks.push_back(entry);
}

LRESULT CALLBACK WndProc::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WndProc* wndProc = WndProc::Get();
	if (wndProc == nullptr)
	{
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	// Call registered wnd proc
	for (auto& wndproc : wndProc->m_registeredCallbacks)
	{
		if (!wndproc.funcAddr)
		{
			continue;
		}

		wndproc.funcAddr(hwnd, uMsg, wParam, lParam, wndproc.object);
	}

	if (wndProc->m_oldWndProc)
	{
		return wndProc->m_oldWndProc(hwnd, uMsg, wParam, lParam);
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}