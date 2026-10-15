#pragma once
#include <windows.h>
#include "config.hpp"
#include "detector.hpp"

namespace inkwell {

    struct ZoneStatus {
        DetectionResult top;
        DetectionResult left;
        DetectionResult right;
        DetectionResult center;
        DetectionResult gank_left;
        DetectionResult gank_right;
        bool bot_active = true;
        bool auto_parry = true;
        float frame_rate = 60.0f;
    };

    enum class DragMode {
        None,
        MoveMenu,
        MoveBox,
        ResizeBox
    };

    class OverlayWindow {
    public:
        OverlayWindow();
        ~OverlayWindow();

        bool Create(HINSTANCE hInstance, Config* pConfig);
        void Update(const ZoneStatus& status, const Config& config);
        void ToggleVisibility();
        void ToggleMenu();
        void SetMenuOpen(bool open);
        bool IsVisible() const { return m_visible; }
        bool IsMenuOpen() const { return m_menuOpen; }
        HWND GetHwnd() const { return m_hwnd; }
        void TriggerCalibrationFlash(const wchar_t* msg = nullptr) {
            m_calibFlash = true;
            m_calibFlashTime = GetTickCount64();
            if (msg) {
                wcsncpy_s(m_calibFlashMsg, msg, _TRUNCATE);
            } else {
                wcscpy_s(m_calibFlashMsg, L"[AUTO-CALIBRATION COMPLETE] Combat Boxes Locked to Stance Widget!");
            }
        }

    private:
        static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
        void Render();
        void HandleLButtonDown(int mouseX, int mouseY);
        void HandleMouseMove(int mouseX, int mouseY);
        void HandleLButtonUp(int mouseX, int mouseY);
        void HandleKeyDown(WPARAM wParam);

        HWND m_hwnd = nullptr;
        bool m_visible = true;
        bool m_menuOpen = true; 
        ZoneStatus m_status = {};
        Config* m_pConfig = nullptr;

        int m_menuX = 30;
        int m_menuY = 40;
        int m_menuW = 860;
        int m_menuH = 740;

        DragMode m_dragMode = DragMode::None;
        int m_dragBoxIdx = -1;
        int m_dragStartX = 0;
        int m_dragStartY = 0;
        int m_origMenuX = 0, m_origMenuY = 0;
        bool m_calibFlash = false;
        uint64_t m_calibFlashTime = 0;
        wchar_t m_calibFlashMsg[160] = L"[AUTO-CALIBRATION COMPLETE] Combat Boxes Locked to Stance Widget!";
    };

}
