#include <windows.h>
#include <timeapi.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <algorithm>
#include <random>
#include "config.hpp"
#include "capture.hpp"
#include "detector.hpp"
#include "input.hpp"
#include "overlay.hpp"

#pragma comment(lib, "winmm.lib")

using namespace inkwell;

static std::mt19937 g_rng(std::random_device{}());

int GetHumanizedDelay(const Config& config, int minMs, int maxMs) {
    if (minMs >= maxMs) return minMs;
    if (config.use_gaussian_distribution) {
        float mean = (minMs + maxMs) / 2.0f;
        std::normal_distribution<float> dist(mean, config.gaussian_stddev_ms);
        float val = dist(g_rng);
        return static_cast<int>(std::clamp(val, static_cast<float>(minMs), static_cast<float>(maxMs)));
    } else {
        std::uniform_int_distribution<int> dist(minMs, maxMs);
        return dist(g_rng);
    }
}

void AutoCalibrate(Config& config, ScreenCapture& capturer, int screenW, int screenH) {
    int scanX = static_cast<int>(screenW * 0.38f);
    int scanY = static_cast<int>(screenH * 0.25f);
    int scanW = static_cast<int>(screenW * 0.24f);
    int scanH = static_cast<int>(screenH * 0.45f);

    if (capturer.CaptureFullCombatRegion(scanX, scanY, scanW, scanH)) {
        const uint32_t* pixels = capturer.GetPixels();
        int width = capturer.GetWidth();
        int height = capturer.GetHeight();
        int stride = capturer.GetAllocWidth();

        int64_t redSumX = 0, redSumY = 0;
        int redCount = 0;

        for (int y = 0; y < height; y += 2) {
            const uint32_t* row = pixels + (y * stride);
            for (int x = 0; x < width; x += 2) {
                uint32_t color = row[x];
                uint8_t b = color & 0xFF;
                uint8_t g = (color >> 8) & 0xFF;
                uint8_t r = (color >> 16) & 0xFF;

                if (r >= 175 && g <= 75 && b <= 75 && (r - g >= 100) && (r - b >= 100)) {
                    redSumX += (scanX + x);
                    redSumY += (scanY + y);
                    redCount++;
                }
            }
        }

        int targetX = screenW / 2;
        int targetY = static_cast<int>(screenH * 0.435f);

        if (redCount > 50) {
            targetX = static_cast<int>(redSumX / redCount);
            targetY = static_cast<int>(redSumY / redCount);
        }

        config.top_zone    = { targetX - 120, targetY - 250, 240, 185 };
        config.left_zone   = { targetX - 240, targetY + 10,  180, 180 };
        config.right_zone  = { targetX + 60,  targetY - 10,  200, 190 };
        config.center_zone = { targetX - 90,  targetY - 60,  180, 120 };
        config.gank_left   = { targetX - 440, targetY - 20,  180, 200 };
        config.gank_right  = { targetX + 260, targetY - 20,  180, 200 };

        config.SaveToFile();
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    timeBeginPeriod(1);

    HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
    if (hUser32) {
        typedef BOOL(WINAPI* SetProcessDpiAwarenessContextProc)(DPI_AWARENESS_CONTEXT);
        auto pSetContext = (SetProcessDpiAwarenessContextProc)GetProcAddress(hUser32, "SetProcessDpiAwarenessContext");
        if (pSetContext) {
            pSetContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
        } else {
            SetProcessDPIAware();
        }
    } else {
        SetProcessDPIAware();
    }

    Config config;
    config.LoadFromFile();

    ScreenCapture capturer;
    OverlayWindow overlay;

    if (!overlay.Create(hInstance, &config)) {
        MessageBoxW(nullptr, L"Failed to create overlay window!", L"Inkwell For Honor Script", MB_ICONERROR);
        return 1;
    }

    bool running = true;
    ZoneStatus status = {};

    uint64_t lastReactionTimestamp = 0;
    
    // Directional Attack Onset Trackers
    bool topActive = false, leftActive = false, rightActive = false;
    uint64_t topStartMs = 0, leftStartMs = 0, rightStartMs = 0;
    bool topStanceSwitched = false, leftStanceSwitched = false, rightStanceSwitched = false;

    static bool f1Was = false, f2Was = false, f3Was = false, f11Was = false;

    MSG msg = {};

    while (running) {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                running = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        if (!running) break;

        // Hotkey Controls
        bool f1Now = (GetAsyncKeyState(VK_F1) & 0x8000) != 0;
        if (f1Now && !f1Was) {
            config.bot_active = !config.bot_active;
            config.SaveToFile();
            if (config.audio_feedback) InputManager::PlayCue(config.bot_active ? 1200 : 600, 45);
        }
        f1Was = f1Now;

        bool f2Now = (GetAsyncKeyState(VK_F2) & 0x8000) != 0;
        if (f2Now && !f2Was) {
            overlay.ToggleVisibility();
        }
        f2Was = f2Now;

        bool f3Now = (GetAsyncKeyState(VK_F3) & 0x8000) != 0;
        if (f3Now && !f3Was) {
            int curScreenW = GetSystemMetrics(SM_CXSCREEN);
            int curScreenH = GetSystemMetrics(SM_CYSCREEN);
            AutoCalibrate(config, capturer, curScreenW, curScreenH);
            overlay.TriggerCalibrationFlash(L"[AUTO-CALIBRATION COMPLETE] All Combat Boxes Locked!");
        }
        f3Was = f3Now;

        bool f11Now = (GetAsyncKeyState(VK_F11) & 0x8000) != 0;
        if (f11Now && !f11Was) {
            overlay.ToggleMenu();
        }
        f11Was = f11Now;

        if (!config.bot_active) {
            status.bot_active = false;
            overlay.Update(status, config);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        status.bot_active = true;

        int screenW = GetSystemMetrics(SM_CXSCREEN);
        int screenH = GetSystemMetrics(SM_CYSCREEN);

        Rect topR    = config.GetScaledRect(config.top_zone, screenW, screenH);
        Rect leftR   = config.GetScaledRect(config.left_zone, screenW, screenH);
        Rect rightR  = config.GetScaledRect(config.right_zone, screenW, screenH);
        Rect centerR = config.GetScaledRect(config.center_zone, screenW, screenH);
        Rect gankLR  = config.GetScaledRect(config.gank_left, screenW, screenH);
        Rect gankRR  = config.GetScaledRect(config.gank_right, screenW, screenH);

        // Compute Bounding Rectangle Wrapping Full Combat Region for Single-Pass Desktop Capture
        int minX = (std::min)({ topR.x, leftR.x, rightR.x, centerR.x, gankLR.x, gankRR.x });
        int minY = (std::min)({ topR.y, leftR.y, rightR.y, centerR.y, gankLR.y, gankRR.y });
        int maxX = (std::max)({ topR.x + topR.w, leftR.x + leftR.w, rightR.x + rightR.w, centerR.x + centerR.w, gankLR.x + gankLR.w, gankRR.x + gankRR.w });
        int maxY = (std::max)({ topR.y + topR.h, leftR.y + leftR.h, rightR.y + rightR.h, centerR.y + centerR.h, gankLR.y + gankLR.h, gankRR.y + gankRR.h });

        int scanX = (std::max)(0, minX - 10);
        int scanY = (std::max)(0, minY - 10);
        int scanW = (std::min)(screenW - scanX, maxX - scanX + 20);
        int scanH = (std::min)(screenH - scanY, maxY - scanY + 20);

        uint64_t nowMs = GetTickCount64();

        // 1. Single-Pass GDI BitBlt Capture (<1ms Latency)
        DetectionResult centerRes = {}, topRes = {}, leftRes = {}, rightRes = {};
        DetectionResult gankLeftRes = {}, gankRightRes = {};

        if (capturer.CaptureFullCombatRegion(scanX, scanY, scanW, scanH)) {
            // Extract SubViews from captured region in memory (O(1) pointer offset)
            SubView centerSV = capturer.GetSubView(centerR.x, centerR.y, centerR.w, centerR.h);
            SubView topSV    = capturer.GetSubView(topR.x, topR.y, topR.w, topR.h);
            SubView leftSV   = capturer.GetSubView(leftR.x, leftR.y, leftR.w, leftR.h);
            SubView rightSV  = capturer.GetSubView(rightR.x, rightR.y, rightR.w, rightR.h);

            centerRes = ColorDetector::AnalyzeSubViewCenter(centerSV, config.red_shield, config.orange_bash, config.nuxia_trap, config.shaman_bleed, false);
            topRes    = ColorDetector::AnalyzeSubViewDirectional(topSV, config.red_indicator, config.white_flash, topActive);
            leftRes   = ColorDetector::AnalyzeSubViewDirectional(leftSV, config.red_indicator, config.white_flash, leftActive);
            rightRes  = ColorDetector::AnalyzeSubViewDirectional(rightSV, config.red_indicator, config.white_flash, rightActive);

            if (config.gank_autoblock_enabled) {
                SubView gankLSV = capturer.GetSubView(gankLR.x, gankLR.y, gankLR.w, gankLR.h);
                SubView gankRSV = capturer.GetSubView(gankRR.x, gankRR.y, gankRR.w, gankRR.h);
                gankLeftRes  = ColorDetector::AnalyzeSubViewGank(gankLSV, config.red_indicator);
                gankRightRes = ColorDetector::AnalyzeSubViewGank(gankRSV, config.red_indicator);
            }
        }

        status.top        = topRes;
        status.left       = leftRes;
        status.right      = rightRes;
        status.center     = centerRes;
        status.gank_left  = gankLeftRes;
        status.gank_right = gankRightRes;
        overlay.Update(status, config);

        auto updateTracker = [&](DetectionResult& res, bool& active, uint64_t& startMs, bool& stanceSwitched) {
            if (res.state != ThreatState::None) {
                if (!active) {
                    active = true;
                    startMs = nowMs;
                    stanceSwitched = false;
                }
            } else {
                if (active && (nowMs - startMs > 750)) {
                    active = false;
                    startMs = 0;
                    stanceSwitched = false;
                }
            }
        };

        updateTracker(topRes, topActive, topStartMs, topStanceSwitched);
        updateTracker(leftRes, leftActive, leftStartMs, leftStanceSwitched);
        updateTracker(rightRes, rightActive, rightStartMs, rightStanceSwitched);

        if (nowMs - lastReactionTimestamp < static_cast<uint64_t>(config.action_cooldown_ms)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        int blockJitter = config.enable_random_delays ? GetHumanizedDelay(config, config.random_block_delay_min_ms, config.random_block_delay_max_ms) : 0;
        int bashJitter  = config.enable_random_delays ? GetHumanizedDelay(config, config.random_dodge_bash_delay_min_ms, config.random_dodge_bash_delay_max_ms) : 0;

        // --- EXECUTOR ARBITRATION MATRIX ---

        // A. Nuxia Trap Counter
        if (config.anti_nuxia_trap && centerRes.state == ThreatState::NuxiaTrap) {
            if (blockJitter > 0) std::this_thread::sleep_for(std::chrono::milliseconds(blockJitter));
            InputManager::SendLightAttack(config.key_hold_ms);
            lastReactionTimestamp = nowMs;
            continue;
        }

        // B. Shaman Bleed Bite
        if (config.shaman_bite_enabled && centerRes.state == ThreatState::ShamanBleed) {
            InputManager::SendShamanBite(config.key_move_up, config.key_dodge, config.key_gb, config.key_hold_ms);
            lastReactionTimestamp = nowMs;
            continue;
        }

        // C. Guardbreak Reaction (Auto CGB / Auto LMB)
        if (centerRes.state == ThreatState::RedShieldGB) {
            if (config.auto_lmb_on_gb) {
                InputManager::SendLightAttack(config.key_hold_ms);
            } else if (config.auto_rmb_black_prior && config.full_block_mode) {
                InputManager::SendHeavyAttack(config.key_hold_ms);
            } else if (config.auto_counter_gb) {
                InputManager::SendGuardbreakTech(config.key_hold_ms, config.key_gb);
            }
            lastReactionTimestamp = nowMs;
            continue;
        }

        // D. Orange Bash Reaction
        if (centerRes.state == ThreatState::OrangeBash) {
            int bashDelay = config.bash_dodge_delay_ms + bashJitter;
            if (bashDelay > 0) std::this_thread::sleep_for(std::chrono::milliseconds(bashDelay));

            if (config.auto_flip_enabled) {
                InputManager::SendFullBlockHold(config.key_fullblock, 25);
                InputManager::SendHeavyAttack(config.key_hold_ms);
            } else if (config.hidden_stance_evasion || config.highlander_stance_dodge) {
                InputManager::SendKey(config.key_fullblock, config.key_hold_ms);
            } else if (config.bash_dodge) {
                WORD sideDir = (config.bash_dodge_side == 1) ? config.key_move_left : config.key_move_right;
                InputManager::SendDirectionalDodge(sideDir, config.key_dodge, config.key_hold_ms);
                if (config.follow_up_dodge_attack) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(config.punish_delay_ms));
                    InputManager::SendLightAttack(config.key_hold_ms);
                }
            }
            lastReactionTimestamp = nowMs;
            continue;
        }

        // E. External Gank AutoBlock
        if (config.gank_autoblock_enabled) {
            if (gankLeftRes.state == ThreatState::RedAttack) {
                if (blockJitter > 0) std::this_thread::sleep_for(std::chrono::milliseconds(blockJitter));
                InputManager::SendKey(config.key_left, config.key_hold_ms);
                lastReactionTimestamp = nowMs;
                continue;
            } else if (gankRightRes.state == ThreatState::RedAttack) {
                if (blockJitter > 0) std::this_thread::sleep_for(std::chrono::milliseconds(blockJitter));
                InputManager::SendKey(config.key_right, config.key_hold_ms);
                lastReactionTimestamp = nowMs;
                continue;
            }
        }

        // F. Decoupled Feint-Proof Directional Arbitration
        auto handleDirectional = [&](ThreatState state, WORD stanceKey, WORD moveDir, bool isTop, bool active, uint64_t startMs, bool& stanceSwitched) {
            if (state == ThreatState::None && !active) return false;

            uint64_t elapsedMs = active ? (nowMs - startMs) : 0;

            // 1. AutoBlock Stance Switch (Frame 1: Guard Alignment for Blockable Attacks)
            if (config.auto_block_enabled && !stanceSwitched && (state == ThreatState::RedAttack || state == ThreatState::ParryFlash)) {
                if (blockJitter > 0) std::this_thread::sleep_for(std::chrono::milliseconds(blockJitter));
                InputManager::SendKey(stanceKey, config.key_hold_ms);
                stanceSwitched = true;
            }

            // 2. UNBLOCKABLE ATTACKS: MANDATORY PARRY ONLY (NEVER DEFLECT / NEVER DODGE LEFT/RIGHT)
            if (state == ThreatState::UnblockableAttack) {
                if (state == ThreatState::ParryFlash || elapsedMs >= static_cast<uint64_t>(config.unblockable_parry_delay_ms) || elapsedMs >= 400) {
                    int parryJitter = config.enable_random_delays ? GetHumanizedDelay(config, 0, config.random_parry_jitter_ms) : 0;
                    if (parryJitter > 0) std::this_thread::sleep_for(std::chrono::milliseconds(parryJitter));

                    InputManager::SendKey(stanceKey, config.key_hold_ms);
                    InputManager::SendHeavyAttack(config.key_hold_ms);
                    lastReactionTimestamp = nowMs;
                    return true;
                }
                return false;
            }

            // 3. STANDARD PARRIES (Confirmed Parry Flash)
            if (config.auto_parry_all && state == ThreatState::ParryFlash) {
                int parryJitter = config.enable_random_delays ? GetHumanizedDelay(config, 0, config.random_parry_jitter_ms) : 0;
                if (parryJitter > 0) std::this_thread::sleep_for(std::chrono::milliseconds(parryJitter));

                InputManager::SendKey(stanceKey, config.key_hold_ms);
                InputManager::SendHeavyAttack(config.key_hold_ms);
                lastReactionTimestamp = nowMs;
                return true;
            }

            // 4. AUTO DEFLECT (Standard Blockable Attacks ONLY)
            if (config.auto_deflect_enabled && state != ThreatState::UnblockableAttack) {
                bool triggerDeflect = false;
                if (state == ThreatState::ParryFlash) {
                    triggerDeflect = true;
                } else if (state == ThreatState::RedAttack && elapsedMs >= 420 && elapsedMs <= 620) {
                    triggerDeflect = true;
                }

                if (triggerDeflect) {
                    if (config.kensei_deflect && isTop) {
                        InputManager::SendDirectionalDodge(config.key_move_up, config.key_dodge, config.key_hold_ms);
                    } else {
                        InputManager::SendDirectionalDodge(moveDir, config.key_dodge, config.key_hold_ms);
                    }
                    if (config.follow_up_deflect_attack) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(config.punish_delay_ms));
                        InputManager::SendLightAttack(config.key_hold_ms);
                    }
                    lastReactionTimestamp = nowMs;
                    return true;
                }
            }

            // 5. CRUSHING COUNTER
            if (state != ThreatState::UnblockableAttack) {
                bool triggerCC = false;
                if (config.warden_crushing_counter && isTop && (state == ThreatState::ParryFlash || (state == ThreatState::RedAttack && elapsedMs >= 380))) {
                    triggerCC = true;
                } else if (config.crushing_counter_all && (state == ThreatState::ParryFlash || (state == ThreatState::RedAttack && elapsedMs >= 380))) {
                    triggerCC = true;
                }

                if (triggerCC) {
                    InputManager::SendKey(stanceKey, config.key_hold_ms);
                    InputManager::SendLightAttack(config.key_hold_ms);
                    lastReactionTimestamp = nowMs;
                    return true;
                }
            }

            return false;
        };

        if (handleDirectional(topRes.state, config.key_top, config.key_move_up, true, topActive, topStartMs, topStanceSwitched)) continue;
        if (handleDirectional(leftRes.state, config.key_left, config.key_move_left, false, leftActive, leftStartMs, leftStanceSwitched)) continue;
        if (handleDirectional(rightRes.state, config.key_right, config.key_move_right, false, rightActive, rightStartMs, rightStanceSwitched)) continue;

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    timeEndPeriod(1);
    return 0;
}
