#include "InputManager.h"
#include "WndProc.h"
#include "Includes.h"
#include <hidusage.h>

InputManager::InputManager()
{}

InputManager* InputManager::Get()
{
    static std::unique_ptr<InputManager> singleton = nullptr;
    if (!singleton)
    {
        singleton = std::unique_ptr<InputManager>(new InputManager());
    }

    return singleton.get();
}

bool InputManager::Initialize(HWND hwnd)
{
    RAWINPUTDEVICE Rid[2];

    // Mouse
    Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
    Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
    Rid[0].dwFlags = RIDEV_INPUTSINK;
    Rid[0].hwndTarget = hwnd;

    // Keyboard
    Rid[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
    Rid[1].usUsage = HID_USAGE_GENERIC_KEYBOARD;    
    Rid[1].dwFlags = RIDEV_INPUTSINK;
    Rid[1].hwndTarget = hwnd; 

    if (RegisterRawInputDevices(Rid, 2, sizeof(RAWINPUTDEVICE)) == FALSE)
    {
        // Registration failed, handle error
    }

    // Mouse idle timer
    SetTimer(hwnd, 1, 10, NULL);

	WndProc::Get()->RegisterWndProc(InputManager::WindowProc, this);
	
    m_hwnd = hwnd;
	return true;
}

InputManager::~InputManager()
{
    KillTimer(m_hwnd, 1);
}

void InputManager::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, void* object)
{
    InputManager* inputMgr = Get();
	switch (uMsg)
	{
    case WM_INPUT:
    {
        UINT dwSize;
        GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
        LPBYTE lpb = new BYTE[dwSize];
        
        if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) == dwSize)
        {
            RAWINPUT* rawInput = (RAWINPUT*)lpb;
            // handle mouse input
            if (rawInput->header.dwType == RIM_TYPEMOUSE)
            {
                // Extract relative movement data for X and Y axes
                const long& mouseX = rawInput->data.mouse.lLastX;
                const long& mouseY = rawInput->data.mouse.lLastY;
				inputMgr->m_inputState.MouseXAxis = (float)mouseX;
                inputMgr->m_inputState.MouseYAxis = (float)mouseY;
                inputMgr->m_lastMouseMoveTime = GetTickCount64();

                std::string msg = "mouseX:" + std::to_string(mouseX) + " mouseY:" + std::to_string(mouseY) + "\n";
                OutputDebugStringA(msg.c_str());
            }
            // handle keyboard input
            else if (rawInput->header.dwType == RIM_TYPEKEYBOARD)
            {
                const USHORT& virtualKeyCode = rawInput->data.keyboard.VKey;
                const USHORT& flags = rawInput->data.keyboard.Flags; // 0 for key down, 1 for key up
                bool isKeyDown = flags == RI_KEY_MAKE;
                float dir = isKeyDown ? 1.0f : -1.0f; // dir for axis
                
                inputMgr->ClearActions();
                // Handle key down event here
                if (virtualKeyCode == 'A')
                {
                    inputMgr->m_inputState.RightAxis = max(-1.0f, inputMgr->m_inputState.RightAxis - 1.0f * dir);
                }
				else if (virtualKeyCode == 'D')
				{
                    inputMgr->m_inputState.RightAxis = min(1.0f, inputMgr->m_inputState.RightAxis + 1.0f * dir);
                }
                else if (virtualKeyCode == 'W')
                {
                    inputMgr->m_inputState.ForwardAxis = min(1.0f, inputMgr->m_inputState.ForwardAxis + 1.0f * dir);
                }
                else if (virtualKeyCode == 'S')
                {
                    inputMgr->m_inputState.ForwardAxis = max(-1.0f, inputMgr->m_inputState.ForwardAxis - 1.0f * dir);
                }
                else if (virtualKeyCode == 'O')
                {
                    if (!isKeyDown)
                    {
                        inputMgr->m_inputState.SwitchCamera = true;
                    }
                }
                else if (virtualKeyCode == 'P')
                {
                    if (!isKeyDown)
                    {
                        inputMgr->m_inputState.SwitchCharacter = true;
                    }
                }
                 
            }

            inputMgr->Update();
        }

        delete[] lpb;
        break;
    }
    case WM_TIMER:
    {
        if (wParam == 1)
        {
            uint64_t dwCurrentTime = GetTickCount64();
            if (dwCurrentTime - inputMgr->m_lastMouseMoveTime > 10) // If idle for more than 500ms
            {
                inputMgr->ClearActions();
                // Mouse has stopped moving
				inputMgr->m_inputState.MouseXAxis = 0.0f;
                inputMgr->m_inputState.MouseYAxis = 0.0f;
                inputMgr->Update();
            }
        }
        break;
    }
	}
}

void InputManager::Update()
{
    if (m_controlObjects.empty())
    {
        return;
    }

    for (auto& controlObj : m_controlObjects)
    {
        if (!controlObj)
        {
            continue;
        }

        controlObj->ProcessInput(m_inputState);
    }
}

void InputManager::ClearActions()
{
    m_inputState.SwitchCamera = false;
    m_inputState.SwitchCharacter = false;
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