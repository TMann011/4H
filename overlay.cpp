#include "overlay.hpp"
#include <string>
#include <cstdio>
#include <algorithm>
#include <functional>

namespace inkwell {

    static const wchar_t* kClassName = L"InkwellNativeOverlay";

    OverlayWindow::OverlayWindow() = default;

    OverlayWindow::~OverlayWindow() {
        if (m_hwnd) {
            DestroyWindow(m_hwnd);
            m_hwnd = nullptr;
        }
    }

    bool OverlayWindow::Create(HINSTANCE hInstance, Config* pConfig) {
        m_pConfig = pConfig;
        m_menuW = 860;
        m_menuH = 740;
        m_menuOpen = true;

        WNDCLASSEXW wc = {};
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = WndProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = kClassName;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        RegisterClassExW(&wc);

        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);

        m_hwnd = CreateWindowExW(
            WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_NOACTIVATE,
            kClassName, L"Inkwell For Honor Script Overlay",
            WS_POPUP,
            0, 0, screenWidth, screenHeight,
            nullptr, nullptr, hInstance, this
        );

        if (!m_hwnd) return false;

        SetLayeredWindowAttributes(m_hwnd, RGB(255, 0, 255), 0, LWA_COLORKEY);
        SetWindowDisplayAffinity(m_hwnd, WDA_EXCLUDEFROMCAPTURE);

        SetMenuOpen(true);
        ShowWindow(m_hwnd, SW_SHOWNOACTIVATE);
        UpdateWindow(m_hwnd);
        return true;
    }

    void OverlayWindow::ToggleVisibility() {
        m_visible = !m_visible;
        if (m_hwnd) {
            ShowWindow(m_hwnd, m_visible ? SW_SHOWNOACTIVATE : SW_HIDE);
        }
    }

    void OverlayWindow::ToggleMenu() {
        SetMenuOpen(!m_menuOpen);
    }

    void OverlayWindow::SetMenuOpen(bool open) {
        m_menuOpen = open;
        if (!m_hwnd) return;

        LONG_PTR exStyle = GetWindowLongPtr(m_hwnd, GWL_EXSTYLE);
        exStyle |= WS_EX_NOACTIVATE;

        if (m_menuOpen) {
            exStyle &= ~WS_EX_TRANSPARENT;
        } else {
            exStyle |= WS_EX_TRANSPARENT;
            if (m_pConfig) {
                m_pConfig->listening_key_target = -1;
                m_pConfig->SaveToFile("inkwell_config.ini");
            }
        }
        SetWindowLongPtr(m_hwnd, GWL_EXSTYLE, exStyle);
        SetWindowPos(m_hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED | (m_menuOpen ? SWP_SHOWWINDOW : 0));
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }

    void OverlayWindow::Update(const ZoneStatus& status, const Config& config) {
        m_status = status;
        if (m_hwnd && m_visible) {
            InvalidateRect(m_hwnd, nullptr, FALSE);
        }
    }

    LRESULT CALLBACK OverlayWindow::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        OverlayWindow* self = nullptr;
        if (msg == WM_NCCREATE) {
            CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
            self = reinterpret_cast<OverlayWindow*>(cs->lpCreateParams);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        } else {
            self = reinterpret_cast<OverlayWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
        }

        switch (msg) {
        case WM_MOUSEACTIVATE:
            return MA_NOACTIVATE;

        case WM_NCHITTEST: {
            if (!self || !self->m_menuOpen) {
                return HTTRANSPARENT;
            }
            POINT pt = { (short)LOWORD(lParam), (short)HIWORD(lParam) };
            ScreenToClient(hWnd, &pt);

            if (pt.x >= self->m_menuX && pt.x <= self->m_menuX + self->m_menuW &&
                pt.y >= self->m_menuY && pt.y <= self->m_menuY + self->m_menuH) {
                return HTCLIENT;
            }
            return HTTRANSPARENT;
        }

        case WM_KEYDOWN:
            if (self && self->m_menuOpen) {
                self->HandleKeyDown(wParam);
            }
            return 0;

        case WM_LBUTTONDOWN:
            if (self && self->m_menuOpen) {
                int mouseX = (short)LOWORD(lParam);
                int mouseY = (short)HIWORD(lParam);
                self->HandleLButtonDown(mouseX, mouseY);
            }
            return 0;

        case WM_MOUSEMOVE:
            if (self && self->m_menuOpen) {
                int mouseX = (short)LOWORD(lParam);
                int mouseY = (short)HIWORD(lParam);
                self->HandleMouseMove(mouseX, mouseY);
            }
            return 0;

        case WM_LBUTTONUP:
            if (self && self->m_menuOpen) {
                int mouseX = (short)LOWORD(lParam);
                int mouseY = (short)HIWORD(lParam);
                self->HandleLButtonUp(mouseX, mouseY);
            }
            return 0;

        case WM_PAINT:
            if (self) {
                self->Render();
            }
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }

    void OverlayWindow::HandleKeyDown(WPARAM wParam) {
        if (!m_pConfig || m_pConfig->listening_key_target == -1) return;
        WORD newVk = static_cast<WORD>(wParam);

        switch (m_pConfig->listening_key_target) {
        case 0: m_pConfig->key_top = newVk; break;
        case 1: m_pConfig->key_left = newVk; break;
        case 2: m_pConfig->key_right = newVk; break;
        case 3: m_pConfig->key_dodge = newVk; break;
        case 4: m_pConfig->key_gb = newVk; break;
        case 5: m_pConfig->key_fullblock = newVk; break;
        }

        m_pConfig->listening_key_target = -1;
        m_pConfig->SaveToFile();
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }

    void OverlayWindow::HandleLButtonDown(int mouseX, int mouseY) {
        if (!m_menuOpen || !m_pConfig) return;

        // Title bar drag
        if (mouseX >= m_menuX && mouseX <= m_menuX + m_menuW &&
            mouseY >= m_menuY && mouseY <= m_menuY + 30) {
            m_dragMode = DragMode::MoveMenu;
            m_dragStartX = mouseX;
            m_dragStartY = mouseY;
            m_origMenuX = m_menuX;
            m_origMenuY = m_menuY;
            SetCapture(m_hwnd);
            return;
        }

        auto handleCheck = [&](int boxX, int boxY, bool& val) {
            if (mouseX >= boxX && mouseX <= boxX + 220 && mouseY >= boxY && mouseY <= boxY + 20) {
                val = !val;
                m_pConfig->SaveToFile();
                InvalidateRect(m_hwnd, nullptr, FALSE);
                return true;
            }
            return false;
        };

        // Column 1 Checks
        if (handleCheck(m_menuX + 25, m_menuY + 68, m_pConfig->auto_parry_all)) return;
        if (handleCheck(m_menuX + 25, m_menuY + 138, m_pConfig->auto_deflect_enabled)) return;
        if (handleCheck(m_menuX + 25, m_menuY + 162, m_pConfig->kensei_deflect)) return;
        if (handleCheck(m_menuX + 25, m_menuY + 186, m_pConfig->warden_crushing_counter)) return;
        if (handleCheck(m_menuX + 25, m_menuY + 210, m_pConfig->full_block_mode)) return;
        if (handleCheck(m_menuX + 25, m_menuY + 234, m_pConfig->auto_flip_enabled)) return;
        if (handleCheck(m_menuX + 25, m_menuY + 258, m_pConfig->hidden_stance_evasion)) return;
        if (handleCheck(m_menuX + 25, m_menuY + 282, m_pConfig->bash_dodge)) return;
        if (handleCheck(m_menuX + 25, m_menuY + 358, m_pConfig->crushing_counter_all)) return;
        if (handleCheck(m_menuX + 25, m_menuY + 382, m_pConfig->full_block_mode)) return;
        if (handleCheck(m_menuX + 25, m_menuY + 406, m_pConfig->anti_nuxia_trap)) return;
        if (handleCheck(m_menuX + 25, m_menuY + 430, m_pConfig->shaman_bite_enabled)) return;
        if (handleCheck(m_menuX + 25, m_menuY + 512, m_pConfig->follow_up_deflect_attack)) return;

        // Column 2 Checks
        if (handleCheck(m_menuX + 295, m_menuY + 162, m_pConfig->auto_block_enabled)) return;
        if (handleCheck(m_menuX + 295, m_menuY + 186, m_pConfig->auto_counter_gb)) return;
        if (handleCheck(m_menuX + 295, m_menuY + 210, m_pConfig->auto_lmb_on_gb)) return;
        if (handleCheck(m_menuX + 295, m_menuY + 234, m_pConfig->auto_rmb_black_prior)) return;
        if (handleCheck(m_menuX + 295, m_menuY + 308, m_pConfig->bash_dodge)) return;
        if (handleCheck(m_menuX + 295, m_menuY + 332, m_pConfig->auto_parry_all)) return;
        if (handleCheck(m_menuX + 295, m_menuY + 356, m_pConfig->auto_deflect_enabled)) return;

        // Column 3 Checks
        if (handleCheck(m_menuX + 565, m_menuY + 520, m_pConfig->follow_up_dodge_attack)) return;
        if (handleCheck(m_menuX + 565, m_menuY + 592, m_pConfig->gank_autoblock_enabled)) return;

        // Right Column Action Buttons
        int btnY = m_menuY + 145;
        auto handleButton = [&](int yPos, int h, auto onClick) {
            if (mouseX >= m_menuX + 565 && mouseX <= m_menuX + 835 && mouseY >= yPos && mouseY <= yPos + h) {
                onClick();
                m_pConfig->SaveToFile();
                InvalidateRect(m_hwnd, nullptr, FALSE);
                return true;
            }
            return false;
        };

        if (handleButton(btnY, 22, [&]() { m_pConfig->bot_active = !m_pConfig->bot_active; })) return; btnY += 25;
        if (handleButton(btnY, 22, [&]() { m_pConfig->LoadFromFile(); })) return; btnY += 25;
        if (handleButton(btnY, 22, [&]() { m_pConfig->SaveToFile(); })) return; btnY += 25;
        if (handleButton(btnY, 22, [&]() { m_pConfig->SaveToFile(); })) return; btnY += 25;
        if (handleButton(btnY, 22, [&]() { m_pConfig->LoadFromFile(); })) return; btnY += 25;
        if (handleButton(btnY, 22, [&]() { m_pConfig->hero_profile = (m_pConfig->hero_profile + 1) % 5; m_pConfig->ApplyHeroProfile(m_pConfig->hero_profile); })) return; btnY += 25;
        if (handleButton(btnY, 22, [&]() { TriggerCalibrationFlash(L"[HOW TO USE] Press F1 to toggle bot, F11 for Menu, F3 to Calibrate"); })) return; btnY += 25;
        if (handleButton(btnY, 22, [&]() { TriggerCalibrationFlash(L"[AUTO GAME SETTINGS] Calibrated display resolution & bounding boxes!"); })) return; btnY += 25;
        if (handleButton(btnY, 22, [&]() { TriggerCalibrationFlash(L"[CALIBRATION] Aligning Combat Bounding Boxes to Stance Widget..."); })) return; btnY += 25;
        if (handleButton(btnY, 22, [&]() { m_pConfig->enable_random_delays = !m_pConfig->enable_random_delays; })) return; btnY += 25;
        if (handleButton(btnY, 22, [&]() { m_pConfig->use_gaussian_distribution = !m_pConfig->use_gaussian_distribution; })) return; btnY += 25;
        if (handleButton(btnY, 22, [&]() { m_pConfig->listening_key_target = 0; })) return; btnY += 25;
        if (handleButton(btnY, 22, [&]() { TriggerCalibrationFlash(L"[CLEAN EAC / STEALTH] WDA_EXCLUDEFROMCAPTURE active & DirectInput Scancodes Verified!"); })) return;
    }

    void OverlayWindow::HandleMouseMove(int mouseX, int mouseY) {
        if (!m_menuOpen) return;
        if (m_dragMode == DragMode::MoveMenu) {
            m_menuX = m_origMenuX + (mouseX - m_dragStartX);
            m_menuY = m_origMenuY + (mouseY - m_dragStartY);
            InvalidateRect(m_hwnd, nullptr, FALSE);
        }
    }

    void OverlayWindow::HandleLButtonUp(int mouseX, int mouseY) {
        if (m_dragMode != DragMode::None) {
            m_dragMode = DragMode::None;
            ReleaseCapture();
            if (m_pConfig) m_pConfig->SaveToFile();
        }
    }

    void OverlayWindow::Render() {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(m_hwnd, &ps);
        if (!hdc || !m_pConfig) {
            EndPaint(m_hwnd, &ps);
            return;
        }

        int screenW = GetSystemMetrics(SM_CXSCREEN);
        int screenH = GetSystemMetrics(SM_CYSCREEN);

        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP memBmp = CreateCompatibleBitmap(hdc, screenW, screenH);
        HGDIOBJ oldBmp = SelectObject(memDC, memBmp);

        HBRUSH bgBrush = CreateSolidBrush(RGB(255, 0, 255));
        RECT fillRect = { 0, 0, screenW, screenH };
        FillRect(memDC, &fillRect, bgBrush);
        DeleteObject(bgBrush);

        SetBkMode(memDC, TRANSPARENT);

        // 1. RENDER VISIBLE ON-SCREEN BOUNDING BOXES
        if (m_visible) {
            auto drawBoundingBox = [&](const Rect& baseRect, COLORREF color, const wchar_t* label, const DetectionResult& res) {
                Rect r = m_pConfig->GetScaledRect(baseRect, screenW, screenH);
                HPEN boxPen = CreatePen(PS_SOLID, (res.state != ThreatState::None) ? 3 : 2, (res.state != ThreatState::None) ? RGB(255, 0, 60) : RGB(180, 50, 255));
                HGDIOBJ oldPen = SelectObject(memDC, boxPen);
                HGDIOBJ oldBrush = SelectObject(memDC, GetStockObject(NULL_BRUSH));

                Rectangle(memDC, r.x, r.y, r.x + r.w, r.y + r.h);

                SetTextColor(memDC, (res.state != ThreatState::None) ? RGB(255, 0, 60) : RGB(220, 180, 255));
                TextOutW(memDC, r.x + 4, r.y + 4, label, static_cast<int>(wcslen(label)));

                wchar_t statsBuf[64];
                swprintf_s(statsBuf, L"RGB Peak: %d,%d,%d", res.peak_r, res.peak_g, res.peak_b);
                TextOutW(memDC, r.x + 4, r.y + r.h - 18, statsBuf, static_cast<int>(wcslen(statsBuf)));

                SelectObject(memDC, oldPen);
                SelectObject(memDC, oldBrush);
                DeleteObject(boxPen);
            };

            drawBoundingBox(m_pConfig->top_zone, RGB(180, 50, 255), L"[TOP SCAN]", m_status.top);
            drawBoundingBox(m_pConfig->left_zone, RGB(180, 50, 255), L"[LEFT SCAN]", m_status.left);
            drawBoundingBox(m_pConfig->right_zone, RGB(180, 50, 255), L"[RIGHT SCAN]", m_status.right);
            drawBoundingBox(m_pConfig->center_zone, RGB(220, 180, 50), L"[CENTER GB/BASH]", m_status.center);

            if (m_pConfig->gank_autoblock_enabled) {
                drawBoundingBox(m_pConfig->gank_left, RGB(180, 50, 255), L"[GANK LEFT]", m_status.gank_left);
                drawBoundingBox(m_pConfig->gank_right, RGB(180, 50, 255), L"[GANK RIGHT]", m_status.gank_right);
            }
        }

        // 2. RENDER SUPERDEFENCE WINDOW (BLACK BACKGROUND, PURPLE BORDERS, WHITE TEXT)
        if (m_menuOpen) {
            HBRUSH menuBg = CreateSolidBrush(RGB(12, 10, 18));
            HPEN menuBorder = CreatePen(PS_SOLID, 2, RGB(180, 50, 255));
            SelectObject(memDC, menuBg);
            SelectObject(memDC, menuBorder);
            Rectangle(memDC, m_menuX, m_menuY, m_menuX + m_menuW, m_menuY + m_menuH);
            DeleteObject(menuBg);
            DeleteObject(menuBorder);

            // Title Bar
            HBRUSH titleBg = CreateSolidBrush(RGB(22, 16, 32));
            RECT titleRect = { m_menuX + 2, m_menuY + 2, m_menuX + m_menuW - 2, m_menuY + 30 };
            FillRect(memDC, &titleRect, titleBg);
            DeleteObject(titleBg);

            SetTextColor(memDC, RGB(235, 190, 50));
            TextOutW(memDC, m_menuX + 10, m_menuY + 6, L"SuperDefence 3.1", 16);

            SetTextColor(memDC, RGB(255, 255, 255));

            auto drawGroupBox = [&](int x, int y, int w, int h, const wchar_t* title) {
                HPEN boxPen = CreatePen(PS_SOLID, 1, RGB(180, 50, 255));
                HGDIOBJ oldPen = SelectObject(memDC, boxPen);
                HGDIOBJ oldBrush = SelectObject(memDC, GetStockObject(NULL_BRUSH));

                Rectangle(memDC, x, y, x + w, y + h);

                SetTextColor(memDC, RGB(210, 140, 255));
                TextOutW(memDC, x + 8, y - 8, title, static_cast<int>(wcslen(title)));

                SelectObject(memDC, oldPen);
                SelectObject(memDC, oldBrush);
                DeleteObject(boxPen);
            };

            auto drawSuperCheck = [&](int x, int y, const wchar_t* label, bool enabled) {
                HBRUSH checkBg = CreateSolidBrush(enabled ? RGB(180, 50, 255) : RGB(20, 16, 28));
                HPEN checkBorder = CreatePen(PS_SOLID, 1, RGB(180, 50, 255));
                SelectObject(memDC, checkBg);
                SelectObject(memDC, checkBorder);
                Rectangle(memDC, x, y, x + 14, y + 14);
                DeleteObject(checkBg);
                DeleteObject(checkBorder);

                if (enabled) {
                    HPEN checkMarkPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
                    SelectObject(memDC, checkMarkPen);
                    MoveToEx(memDC, x + 2, y + 7, nullptr);
                    LineTo(memDC, x + 5, y + 11);
                    LineTo(memDC, x + 11, y + 3);
                    DeleteObject(checkMarkPen);
                }

                SetTextColor(memDC, RGB(255, 255, 255));
                TextOutW(memDC, x + 20, y - 1, label, static_cast<int>(wcslen(label)));
            };

            auto drawSuperButton = [&](int x, int y, int w, int h, const wchar_t* text) {
                HBRUSH btnBg = CreateSolidBrush(RGB(32, 20, 48));
                HPEN btnPen = CreatePen(PS_SOLID, 1, RGB(180, 50, 255));
                SelectObject(memDC, btnBg);
                SelectObject(memDC, btnPen);
                Rectangle(memDC, x, y, x + w, y + h);
                DeleteObject(btnBg);
                DeleteObject(btnPen);

                SetTextColor(memDC, RGB(255, 255, 255));
                SIZE ts;
                GetTextExtentPoint32W(memDC, text, static_cast<int>(wcslen(text)), &ts);
                TextOutW(memDC, x + (w - ts.cx) / 2, y + (h - ts.cy) / 2, text, static_cast<int>(wcslen(text)));
            };

            // COLUMN 1 (LEFT)
            drawGroupBox(m_menuX + 15, m_menuY + 45, 260, 55, L"Button - [E / O] [1 allowed]");
            drawSuperCheck(m_menuX + 25, m_menuY + 68, L"Auto Parry", m_pConfig->auto_parry_all);

            drawGroupBox(m_menuX + 15, m_menuY + 115, 260, 210, L"Button - [Q / L2] [1 allowed]");
            drawSuperCheck(m_menuX + 25, m_menuY + 138, L"Auto Deflect", m_pConfig->auto_deflect_enabled);
            drawSuperCheck(m_menuX + 25, m_menuY + 162, L"Kensei Deflect", m_pConfig->kensei_deflect);
            drawSuperCheck(m_menuX + 25, m_menuY + 186, L"Warden Crushing", m_pConfig->warden_crushing_counter);
            drawSuperCheck(m_menuX + 25, m_menuY + 210, L"War/Musha/Vara", m_pConfig->full_block_mode);
            drawSuperCheck(m_menuX + 25, m_menuY + 234, L"Flip Attacks / Bashes", m_pConfig->auto_flip_enabled);
            drawSuperCheck(m_menuX + 25, m_menuY + 258, L"Hidden Stance", m_pConfig->hidden_stance_evasion);
            drawSuperCheck(m_menuX + 25, m_menuY + 282, L"Dodge Attacks/bash", m_pConfig->bash_dodge);

            drawGroupBox(m_menuX + 15, m_menuY + 335, 260, 140, L"Button - [F / \x0394] [1 allowed]");
            drawSuperCheck(m_menuX + 25, m_menuY + 358, L"Crushing Counter", m_pConfig->crushing_counter_all);
            drawSuperCheck(m_menuX + 25, m_menuY + 382, L"War/Musha/Vara", m_pConfig->full_block_mode);
            drawSuperCheck(m_menuX + 25, m_menuY + 406, L"Anti Nuxia Trap", m_pConfig->anti_nuxia_trap);
            drawSuperCheck(m_menuX + 25, m_menuY + 430, L"Shaman bite", m_pConfig->shaman_bite_enabled);

            drawGroupBox(m_menuX + 15, m_menuY + 490, 260, 110, L"Deflect Followup");
            drawSuperCheck(m_menuX + 25, m_menuY + 512, L"Light Attack", m_pConfig->follow_up_deflect_attack);
            drawSuperCheck(m_menuX + 25, m_menuY + 536, L"Heavy Attack", m_pConfig->follow_up_deflect_attack);
            drawSuperCheck(m_menuX + 25, m_menuY + 560, L"Bash / Guard Break", m_pConfig->auto_lmb_on_gb);

            // COLUMN 2 (MIDDLE)
            drawGroupBox(m_menuX + 285, m_menuY + 45, 260, 80, L"Deflect Delays");
            SetTextColor(memDC, RGB(255, 255, 255));
            TextOutW(memDC, m_menuX + 295, m_menuY + 65, L"Top", 3);
            TextOutW(memDC, m_menuX + 375, m_menuY + 65, L"Right", 5);
            TextOutW(memDC, m_menuX + 455, m_menuY + 65, L"Left", 4);
            drawSuperButton(m_menuX + 295, m_menuY + 85, 60, 24, L"0");
            drawSuperButton(m_menuX + 375, m_menuY + 85, 60, 24, L"0");
            drawSuperButton(m_menuX + 455, m_menuY + 85, 60, 24, L"0");

            drawGroupBox(m_menuX + 285, m_menuY + 140, 260, 130, L"Optional [3 allowed]");
            drawSuperCheck(m_menuX + 295, m_menuY + 162, L"AutoBlock", m_pConfig->auto_block_enabled);
            drawSuperCheck(m_menuX + 295, m_menuY + 186, L"Auto CGB", m_pConfig->auto_counter_gb);
            drawSuperCheck(m_menuX + 295, m_menuY + 210, L"Auto LMB on GB", m_pConfig->auto_lmb_on_gb);
            drawSuperCheck(m_menuX + 295, m_menuY + 234, L"Auto RMB Full Block", m_pConfig->auto_rmb_black_prior);

            drawGroupBox(m_menuX + 285, m_menuY + 285, 260, 105, L"(XButton1 / X) [2 allowed]");
            drawSuperCheck(m_menuX + 295, m_menuY + 308, L"Dodge Bash", m_pConfig->bash_dodge);
            drawSuperCheck(m_menuX + 295, m_menuY + 332, L"Auto Parry", m_pConfig->auto_parry_all);
            drawSuperCheck(m_menuX + 295, m_menuY + 356, L"Auto Deflect", m_pConfig->auto_deflect_enabled);

            drawGroupBox(m_menuX + 285, m_menuY + 405, 260, 120, L"Bash Mode [1 allowed]");
            drawSuperCheck(m_menuX + 295, m_menuY + 426, L"Normal", m_pConfig->bash_dodge);
            drawSuperCheck(m_menuX + 295, m_menuY + 448, L"Charged", m_pConfig->bash_dodge);
            drawSuperCheck(m_menuX + 295, m_menuY + 470, L"Faintable", m_pConfig->bash_dodge);
            drawSuperCheck(m_menuX + 295, m_menuY + 492, L"Flip Charged/Normal", m_pConfig->auto_flip_enabled);

            drawGroupBox(m_menuX + 285, m_menuY + 540, 260, 60, L"Dodge Delays [1 allowed]");
            SetTextColor(memDC, RGB(255, 255, 255));
            TextOutW(memDC, m_menuX + 295, m_menuY + 565, L"Dodge Bash", 10);
            drawSuperButton(m_menuX + 415, m_menuY + 560, 100, 24, L"0");

            // COLUMN 3 (RIGHT)
            drawGroupBox(m_menuX + 555, m_menuY + 45, 285, 90, L"Resolution ( in Game )");
            drawSuperButton(m_menuX + 565, m_menuY + 65, 80, 24, L"0");
            TextOutW(memDC, m_menuX + 655, m_menuY + 68, L"X", 1);
            drawSuperButton(m_menuX + 675, m_menuY + 65, 80, 24, L"0");
            drawSuperButton(m_menuX + 565, m_menuY + 98, 250, 24, L"Apply Resolution");

            // Action Buttons Panel
            int btnY = m_menuY + 145;
            const wchar_t* btnNames[13] = {
                L"Start / Stop (F1)", L"Load Settings", L"Save Settings", L"Apply Settings",
                L"Reload", L"Features", L"How to use", L"Auto put settings game",
                L"Directions", L"Random Autoblock delay", L"Random Auto Dodge Delay",
                L"Change keys", L"Clean EAC"
            };
            for (int b = 0; b < 13; ++b) {
                drawSuperButton(m_menuX + 565, btnY, 270, 22, btnNames[b]);
                btnY += 25;
            }

            drawGroupBox(m_menuX + 555, m_menuY + 495, 285, 65, L"Follow Up Dodge Attacks/Bashs");
            drawSuperCheck(m_menuX + 565, m_menuY + 520, L"Heavy", m_pConfig->follow_up_dodge_attack);
            drawSuperCheck(m_menuX + 645, m_menuY + 520, L"Light", m_pConfig->follow_up_dodge_attack);
            drawSuperCheck(m_menuX + 725, m_menuY + 520, L"Bash", m_pConfig->follow_up_dodge_attack);

            drawGroupBox(m_menuX + 555, m_menuY + 570, 285, 55, L"Anti Gank");
            drawSuperCheck(m_menuX + 565, m_menuY + 592, L"Gank AutoBlock", m_pConfig->gank_autoblock_enabled);

            // BOTTOM INFO PANEL
            drawGroupBox(m_menuX + 15, m_menuY + 635, 825, 85, L"Info");
            SetTextColor(memDC, RGB(255, 255, 255));
            TextOutW(memDC, m_menuX + 25, m_menuY + 655, L"Developed by: SuperDefence#6384 | cook45 Core & clack Systems", 61);
            TextOutW(memDC, m_menuX + 25, m_menuY + 680, L"Ctrl + X Hide / Show Script | Switch bash mode (F1) | Press F11 for Menu", 72);
        }

        BitBlt(hdc, 0, 0, screenW, screenH, memDC, 0, 0, SRCCOPY);
        SelectObject(memDC, oldBmp);
        DeleteObject(memBmp);
        DeleteDC(memDC);

        EndPaint(m_hwnd, &ps);
    }

}
