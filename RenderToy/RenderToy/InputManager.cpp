#include "InputManager.h"
#include "WndProc.h"
#include "Includes.h"

bool InputManager::Initialize(HWND hwnd)
{
	WndProc::Get()->RegisterWndProc(InputManager::WindowProc, this);

	m_keyboard = std::make_unique<Keyboard>();
	m_mouse = std::make_unique<Mouse>();
    m_mouse->SetWindow(hwnd);
    m_mouse->SetMode(DirectX::Mouse::MODE_RELATIVE);
	
	return true;
}

InputManager::~InputManager()
{

}

void InputManager::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, void* object)
{
	switch (uMsg)
	{
    case WM_ACTIVATEAPP:
        DirectX::Keyboard::ProcessMessage(uMsg, wParam, lParam);
        DirectX::Mouse::ProcessMessage(uMsg, wParam, lParam);
        break;

    case WM_INPUT:
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MOUSEWHEEL:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
    case WM_MOUSEHOVER:
        DirectX::Mouse::ProcessMessage(uMsg, wParam, lParam);
        break;

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP:
        DirectX::Keyboard::ProcessMessage(uMsg, wParam, lParam);
        break;

	}
}

void InputManager::Update(float delta)
{
    if (!m_controlObjects.empty())
    {
        return;
    }

    for (auto& controlObj : m_controlObjects)
    {
        if (controlObj)
        {
            continue;
        }

        DirectX::Mouse::State mouseState = m_mouse->GetState();
        DirectX::Keyboard::State keyboardState = m_keyboard->GetState();

        controlObj->ProcessInput(mouseState, keyboardState, delta);
    }
}

void InputManager::AddControlObject(IControllable* controlObject)
{
    if (m_controlObjects.contains(controlObject))
    {
        return;
    }

    m_controlObjects.insert(controlObject);
}

void InputManager::RemoveControlObject(IControllable* controlObject)
{
    if (m_controlObjects.contains(controlObject))
    {
        m_controlObjects.erase(controlObject);
    }
}