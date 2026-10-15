#pragma once
#include <windows.h>

namespace inkwell {

    class InputManager {
    public:
        static void SendKey(WORD vkCode, int hold_ms);
        static void SendClick(bool right_click, int hold_ms);
        static void SendLightAttack(int hold_ms);
        static void SendHeavyAttack(int hold_ms);
        static void SendZoneAttack(int hold_ms);
        static void SendMiddleClick(int hold_ms);
        static void SendMouse5(int hold_ms);
        static void SendGuardbreakTech(int hold_ms, WORD extraKey = 'X');
        static void SendDirectionalDodge(WORD dirKey, WORD dodgeKey, int hold_ms);
        static void SendFullBlockHold(WORD fullBlockKey, int duration_ms);
        static void SendShamanBite(WORD moveUpKey, WORD dodgeKey, WORD gbKey, int hold_ms);
        static void SendMouseFlick(int dx, int dy);
        static void PlayCue(int freq, int duration_ms);
    };

}
