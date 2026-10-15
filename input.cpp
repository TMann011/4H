#include "input.hpp"
#include <thread>
#include <chrono>

namespace inkwell {

    void InputManager::SendKey(WORD vkCode, int hold_ms) {
        UINT scanCode = MapVirtualKeyW(vkCode, MAPVK_VK_TO_VSC);

        INPUT inputs[2] = {};
        inputs[0].type = INPUT_KEYBOARD;
        inputs[0].ki.wVk = vkCode;
        inputs[0].ki.wScan = static_cast<WORD>(scanCode);
        inputs[0].ki.dwFlags = KEYEVENTF_SCANCODE;

        inputs[1].type = INPUT_KEYBOARD;
        inputs[1].ki.wVk = vkCode;
        inputs[1].ki.wScan = static_cast<WORD>(scanCode);
        inputs[1].ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;

        SendInput(1, &inputs[0], sizeof(INPUT));
        int actualHold = (hold_ms > 0) ? hold_ms : 12;
        std::this_thread::sleep_for(std::chrono::milliseconds(actualHold));
        SendInput(1, &inputs[1], sizeof(INPUT));
    }

    void InputManager::SendClick(bool right_click, int hold_ms) {
        DWORD downFlag = right_click ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_LEFTDOWN;
        DWORD upFlag   = right_click ? MOUSEEVENTF_RIGHTUP   : MOUSEEVENTF_LEFTUP;

        INPUT input = {};
        input.type = INPUT_MOUSE;

        input.mi.dwFlags = downFlag;
        SendInput(1, &input, sizeof(INPUT));

        int actualHold = (hold_ms > 0) ? hold_ms : 15;
        std::this_thread::sleep_for(std::chrono::milliseconds(actualHold));

        input.mi.dwFlags = upFlag;
        SendInput(1, &input, sizeof(INPUT));
    }

    void InputManager::SendLightAttack(int hold_ms) {
        SendClick(false, hold_ms);
    }

    void InputManager::SendHeavyAttack(int hold_ms) {
        SendClick(true, hold_ms);
    }

    void InputManager::SendZoneAttack(int hold_ms) {
        INPUT inputs[2] = {};
        inputs[0].type = INPUT_MOUSE;
        inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        inputs[1].type = INPUT_MOUSE;
        inputs[1].mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
        SendInput(2, inputs, sizeof(INPUT));

        int actualHold = (hold_ms > 0) ? hold_ms : 15;
        std::this_thread::sleep_for(std::chrono::milliseconds(actualHold));

        inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTUP;
        inputs[1].mi.dwFlags = MOUSEEVENTF_RIGHTUP;
        SendInput(2, inputs, sizeof(INPUT));
    }

    void InputManager::SendMiddleClick(int hold_ms) {
        INPUT input = {};
        input.type = INPUT_MOUSE;

        input.mi.dwFlags = MOUSEEVENTF_MIDDLEDOWN;
        SendInput(1, &input, sizeof(INPUT));

        int actualHold = (hold_ms > 0) ? hold_ms : 15;
        std::this_thread::sleep_for(std::chrono::milliseconds(actualHold));

        input.mi.dwFlags = MOUSEEVENTF_MIDDLEUP;
        SendInput(1, &input, sizeof(INPUT));
    }

    void InputManager::SendMouse5(int hold_ms) {
        INPUT input = {};
        input.type = INPUT_MOUSE;
        input.mi.mouseData = XBUTTON2;

        input.mi.dwFlags = MOUSEEVENTF_XDOWN;
        SendInput(1, &input, sizeof(INPUT));

        int actualHold = (hold_ms > 0) ? hold_ms : 15;
        std::this_thread::sleep_for(std::chrono::milliseconds(actualHold));

        input.mi.dwFlags = MOUSEEVENTF_XUP;
        SendInput(1, &input, sizeof(INPUT));
    }

    void InputManager::SendGuardbreakTech(int hold_ms, WORD extraKey) {
        INPUT mouseInputs[2] = {};
        mouseInputs[0].type = INPUT_MOUSE;
        mouseInputs[0].mi.dwFlags = MOUSEEVENTF_MIDDLEDOWN;

        mouseInputs[1].type = INPUT_MOUSE;
        mouseInputs[1].mi.mouseData = XBUTTON2;
        mouseInputs[1].mi.dwFlags = MOUSEEVENTF_XDOWN;
        SendInput(2, mouseInputs, sizeof(INPUT));

        if (extraKey != 0) {
            UINT scanCode = MapVirtualKeyW(extraKey, MAPVK_VK_TO_VSC);
            INPUT keyInput = {};
            keyInput.type = INPUT_KEYBOARD;
            keyInput.ki.wVk = extraKey;
            keyInput.ki.wScan = static_cast<WORD>(scanCode);
            keyInput.ki.dwFlags = KEYEVENTF_SCANCODE;
            SendInput(1, &keyInput, sizeof(INPUT));
        }

        int actualHold = (hold_ms > 0) ? hold_ms : 15;
        std::this_thread::sleep_for(std::chrono::milliseconds(actualHold));

        mouseInputs[0].mi.dwFlags = MOUSEEVENTF_MIDDLEUP;
        mouseInputs[1].mi.mouseData = XBUTTON2;
        mouseInputs[1].mi.dwFlags = MOUSEEVENTF_XUP;
        SendInput(2, mouseInputs, sizeof(INPUT));

        if (extraKey != 0) {
            UINT scanCode = MapVirtualKeyW(extraKey, MAPVK_VK_TO_VSC);
            INPUT keyInput = {};
            keyInput.type = INPUT_KEYBOARD;
            keyInput.ki.wVk = extraKey;
            keyInput.ki.wScan = static_cast<WORD>(scanCode);
            keyInput.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
            SendInput(1, &keyInput, sizeof(INPUT));
        }
    }

    void InputManager::SendDirectionalDodge(WORD dirKey, WORD dodgeKey, int hold_ms) {
        UINT dirScan   = MapVirtualKeyW(dirKey, MAPVK_VK_TO_VSC);
        UINT dodgeScan = MapVirtualKeyW(dodgeKey, MAPVK_VK_TO_VSC);

        INPUT dirDown = {};
        dirDown.type = INPUT_KEYBOARD;
        dirDown.ki.wVk = dirKey;
        dirDown.ki.wScan = static_cast<WORD>(dirScan);
        dirDown.ki.dwFlags = KEYEVENTF_SCANCODE;
        SendInput(1, &dirDown, sizeof(INPUT));

        std::this_thread::sleep_for(std::chrono::milliseconds(4));

        INPUT dodgeDown = {};
        dodgeDown.type = INPUT_KEYBOARD;
        dodgeDown.ki.wVk = dodgeKey;
        dodgeDown.ki.wScan = static_cast<WORD>(dodgeScan);
        dodgeDown.ki.dwFlags = KEYEVENTF_SCANCODE;
        SendInput(1, &dodgeDown, sizeof(INPUT));

        int actualHold = (hold_ms > 0) ? hold_ms : 15;
        std::this_thread::sleep_for(std::chrono::milliseconds(actualHold));

        INPUT keysUp[2] = {};
        keysUp[0].type = INPUT_KEYBOARD;
        keysUp[0].ki.wVk = dodgeKey;
        keysUp[0].ki.wScan = static_cast<WORD>(dodgeScan);
        keysUp[0].ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;

        keysUp[1].type = INPUT_KEYBOARD;
        keysUp[1].ki.wVk = dirKey;
        keysUp[1].ki.wScan = static_cast<WORD>(dirScan);
        keysUp[1].ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;

        SendInput(2, keysUp, sizeof(INPUT));
    }

    void InputManager::SendFullBlockHold(WORD fullBlockKey, int duration_ms) {
        UINT scanCode = MapVirtualKeyW(fullBlockKey, MAPVK_VK_TO_VSC);

        INPUT keyDown = {};
        keyDown.type = INPUT_KEYBOARD;
        keyDown.ki.wVk = fullBlockKey;
        keyDown.ki.wScan = static_cast<WORD>(scanCode);
        keyDown.ki.dwFlags = KEYEVENTF_SCANCODE;
        SendInput(1, &keyDown, sizeof(INPUT));

        std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));

        INPUT keyUp = {};
        keyUp.type = INPUT_KEYBOARD;
        keyUp.ki.wVk = fullBlockKey;
        keyUp.ki.wScan = static_cast<WORD>(scanCode);
        keyUp.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
        SendInput(1, &keyUp, sizeof(INPUT));
    }

    void InputManager::SendShamanBite(WORD moveUpKey, WORD dodgeKey, WORD gbKey, int hold_ms) {
        SendDirectionalDodge(moveUpKey, dodgeKey, hold_ms);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        SendKey(gbKey, hold_ms);
    }

    void InputManager::SendMouseFlick(int dx, int dy) {
        INPUT input = {};
        input.type = INPUT_MOUSE;
        input.mi.dx = dx;
        input.mi.dy = dy;
        input.mi.dwFlags = MOUSEEVENTF_MOVE;
        SendInput(1, &input, sizeof(INPUT));
    }

    void InputManager::PlayCue(int freq, int duration_ms) {
        std::thread([freq, duration_ms]() {
            Beep(freq, duration_ms);
        }).detach();
    }

}
