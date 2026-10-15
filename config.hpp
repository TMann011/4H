#pragma once
#include <windows.h>
#include <cstdint>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

namespace inkwell {

    struct Rect {
        int x;
        int y;
        int w;
        int h;
    };

    struct ColorFilter {
        uint8_t min_r = 210;
        uint8_t max_r = 255;
        uint8_t min_g = 0;
        uint8_t max_g = 55;
        uint8_t min_b = 0;
        uint8_t max_b = 55;
        int min_contrast = 140; 
        float min_ratio = 0.04f; 
    };

    enum class CombatMode {
        BlockOnly,            
        PureParryOnly,        
        HybridBlockParry,     
        CrushingCounter,      
        DeflectDodge,         
        WardenSpecial,        
        KenseiSpecial,        
        BlackPriorBulwark,    
        HiddenStanceEvasion,  
        FullBlockGuard        
    };

    enum class AutoPunishMode {
        None,             
        LightPunish,      
        HeavyPunish,      
        ZonePunish        
    };

    struct TimingProfile {
        const wchar_t* name;
        int parry_delay_ms;
        int unblockable_parry_delay_ms;
        int counter_gb_delay_ms;
        int bash_dodge_delay_ms;
        int min_attack_duration_for_parry_ms;
        int post_parry_cooldown_ms;
        int post_block_cooldown_ms;
        int action_cooldown_ms;
        int punish_delay_ms;
    };

    static inline TimingProfile GetHeroProfileData(int profileIdx) {
        switch (profileIdx) {
        case 0: return { L"Assassin (Fast Lights ~400ms)", 0, 400, 0, 0, 40, 400, 200, 40, 120 };
        case 1: return { L"Balanced Hybrid (Standard)", 0, 450, 0, 0, 50, 450, 250, 40, 140 };
        case 2: return { L"Heavy / Hyperarmor (Charged)", 20, 500, 0, 50, 80, 500, 300, 50, 160 };
        case 3: return { L"Vanguard (Warden / Kensei)", 0, 420, 0, 0, 40, 420, 220, 40, 130 };
        case 4: return { L"Outlander / Special (Varangian)", 0, 400, 0, 0, 40, 400, 200, 40, 120 };
        default: return { L"Custom User Defined", 0, 450, 0, 0, 40, 450, 250, 40, 140 };
        }
    }

    struct Config {
        // Combat Bounding Boxes
        Rect top_zone    = { 800, 200, 240, 185 }; 
        Rect left_zone   = { 720, 470, 180, 180 }; 
        Rect right_zone  = { 1020, 460, 200, 190 }; 
        Rect center_zone = { 840, 442, 180, 110 }; 
        Rect gank_left   = { 520, 450, 180, 200 }; 
        Rect gank_right  = { 1220, 450, 180, 200 }; 

        int selected_zone_idx = 3; 

        void ResetSelectedZoneToDefault() {
            if (selected_zone_idx == 0) top_zone = { 800, 200, 240, 185 };
            else if (selected_zone_idx == 1) left_zone = { 720, 470, 180, 180 };
            else if (selected_zone_idx == 2) right_zone = { 1020, 460, 200, 190 };
            else if (selected_zone_idx == 3) center_zone = { 840, 442, 180, 110 };
        }

        void CenterSelectedZone(int screenW, int screenH) {
            int cx = (screenW > 0) ? (screenW / 2) : 960;
            int cy = (screenH > 0) ? static_cast<int>(screenH * 0.435f) : 470;
            if (selected_zone_idx == 0) {
                top_zone.x = cx - (top_zone.w / 2);
                top_zone.y = cy - 250;
            } else if (selected_zone_idx == 1) {
                left_zone.x = cx - 240;
                left_zone.y = cy + 10;
            } else if (selected_zone_idx == 2) {
                right_zone.x = cx + 60;
                right_zone.y = cy - 10;
            } else if (selected_zone_idx == 3) {
                center_zone.x = cx - 90;
                center_zone.y = cy - 30;
                center_zone.w = 180;
                center_zone.h = 110;
            }
        }

        // Chromatic Analysis Filters
        ColorFilter red_indicator = { 180, 255, 0, 65, 0, 65, 115, 0.015f }; 
        ColorFilter white_flash   = { 235, 255, 235, 255, 235, 255, 120, 0.0035f }; 
        ColorFilter orange_bash   = { 195, 255, 75, 180, 0, 60, 35, 0.038f };       
        ColorFilter red_shield    = { 175, 255, 0, 65, 0, 65, 115, 0.050f };        
        ColorFilter nuxia_trap    = { 200, 255, 120, 210, 0, 70, 60, 0.025f };      
        ColorFilter shaman_bleed  = { 160, 255, 0, 40, 40, 120, 100, 0.010f };     

        // Keybinds (Customizable Virtual Key Codes)
        WORD key_top        = 'K';
        WORD key_left       = 'J';
        WORD key_right      = 'L';
        WORD key_dodge      = VK_SPACE;
        WORD key_move_up    = 'W';
        WORD key_move_left  = 'A';
        WORD key_move_right = 'D';
        WORD key_gb         = 'X'; 
        WORD key_fullblock  = 'C';
        WORD key_light      = VK_LBUTTON;
        WORD key_heavy      = VK_RBUTTON;

        // Timings & Ultra-Fast Sub-Millisecond Response Baselines
        int input_delay_ms                   = 0;
        int parry_delay_ms                   = 0;   // Instant sub-ms parry dispatch
        int unblockable_parry_delay_ms       = 450; 
        int counter_gb_delay_ms              = 0;   // Instant CGB
        int bash_dodge_delay_ms              = 0;   
        int min_attack_duration_for_parry_ms = 40;  // Ultra-fast 40ms attack onset threshold
        int key_hold_ms                      = 25;  
        int action_cooldown_ms               = 40;  // Ultra-short 40ms lockout so stance switch -> parry transitions instantly
        int post_parry_cooldown_ms           = 400; 
        int post_block_cooldown_ms           = 200; 
        int punish_delay_ms                  = 140; 
        int hero_profile                     = 1;   
        int bash_dodge_side                  = 0;   

        // Humanization Controls (Disabled by default for maximum competitive reaction speed)
        bool enable_random_delays            = false;
        bool use_gaussian_distribution       = false; 
        int random_block_delay_min_ms        = 0;
        int random_block_delay_max_ms        = 15;
        int random_dodge_bash_delay_min_ms   = 0;
        int random_dodge_bash_delay_max_ms   = 20;
        int random_parry_jitter_ms           = 0;
        float gaussian_stddev_ms             = 4.0f;  

        // Decoupled Feature Toggles (Clean Competitive Defaults)
        bool bot_active                      = true;
        bool auto_block_enabled              = true;  // Instant guard switch on red chevron
        bool gank_autoblock_enabled          = true;  
        bool auto_counter_gb                 = true;  // Instant CGB
        bool auto_parry_all                  = true;  // Feint-safe parry on white flash
        bool feint_safe_mode                 = true;  // 100% Feint proof (never clicks RMB on red swing)
        bool auto_lmb_on_gb                  = false; // Off by default to avoid weird attacks
        bool auto_rmb_black_prior            = false; 
        bool auto_deflect_enabled            = false; 
        bool bash_dodge                      = true;  
        bool auto_flip_enabled               = false; 
        bool warden_crushing_counter         = false; 
        bool kensei_deflect                  = false; 
        bool hidden_stance_evasion           = false; 
        bool highlander_stance_dodge         = false; 
        bool shaman_bite_enabled             = false; 
        bool anti_nuxia_trap                 = false; 
        bool full_block_mode                 = false; 
        bool crushing_counter_all            = false; // Off by default to prevent early light swings ("red swings")
        bool follow_up_dodge_attack          = false; 
        bool follow_up_deflect_attack        = false; 
        bool parry_unblockables_only         = false; 
        bool use_mouse5_for_gb               = true;  
        bool show_overlay                    = true;
        bool use_mouse_flick_guard           = true;  
        bool audio_feedback                  = false; 

        int active_tab                       = 0;     
        int listening_key_target             = -1;    

        CombatMode mode                      = CombatMode::HybridBlockParry;
        AutoPunishMode auto_punish           = AutoPunishMode::None;

        Rect GetScaledRect(const Rect& base, int screenWidth, int screenHeight) const {
            if (screenWidth <= 0 || screenHeight <= 0) return base;
            float scaleX = static_cast<float>(screenWidth) / 1920.0f;
            float scaleY = static_cast<float>(screenHeight) / 1080.0f;
            return {
                static_cast<int>(base.x * scaleX),
                static_cast<int>(base.y * scaleY),
                static_cast<int>(base.w * scaleX),
                static_cast<int>(base.h * scaleY)
            };
        }

        void ApplyHeroProfile(int profileIdx) {
            hero_profile = profileIdx;
            TimingProfile p = GetHeroProfileData(profileIdx);
            parry_delay_ms                   = p.parry_delay_ms;
            unblockable_parry_delay_ms       = p.unblockable_parry_delay_ms;
            counter_gb_delay_ms              = p.counter_gb_delay_ms;
            bash_dodge_delay_ms              = p.bash_dodge_delay_ms;
            min_attack_duration_for_parry_ms = p.min_attack_duration_for_parry_ms;
            post_parry_cooldown_ms           = p.post_parry_cooldown_ms;
            post_block_cooldown_ms           = p.post_block_cooldown_ms;
            action_cooldown_ms               = p.action_cooldown_ms;
            punish_delay_ms                  = p.punish_delay_ms;
            SaveToFile();
        }

        static std::string GetConfigFilePath() {
            wchar_t exePath[MAX_PATH] = {};
            GetModuleFileNameW(nullptr, exePath, MAX_PATH);
            std::wstring ws(exePath);
            size_t slash = ws.find_last_of(L"\\/");
            std::wstring dir = (slash != std::wstring::npos) ? ws.substr(0, slash + 1) : L"";
            std::wstring fullW = dir + L"inkwell_config.ini";
            char fullA[MAX_PATH * 2] = {};
            WideCharToMultiByte(CP_UTF8, 0, fullW.c_str(), -1, fullA, sizeof(fullA), nullptr, nullptr);
            return std::string(fullA);
        }

        void SaveToFile(const std::string& path = "") const {
            std::string target = path.empty() ? GetConfigFilePath() : path;
            std::ofstream out(target);
            if (!out.is_open()) return;
            out << "[Zones]\n";
            out << "top=" << top_zone.x << "," << top_zone.y << "," << top_zone.w << "," << top_zone.h << "\n";
            out << "left=" << left_zone.x << "," << left_zone.y << "," << left_zone.w << "," << left_zone.h << "\n";
            out << "right=" << right_zone.x << "," << right_zone.y << "," << right_zone.w << "," << right_zone.h << "\n";
            out << "center=" << center_zone.x << "," << center_zone.y << "," << center_zone.w << "," << center_zone.h << "\n";
            out << "gank_left=" << gank_left.x << "," << gank_left.y << "," << gank_left.w << "," << gank_left.h << "\n";
            out << "gank_right=" << gank_right.x << "," << gank_right.y << "," << gank_right.w << "," << gank_right.h << "\n";

            out << "[Timings]\n";
            out << "hero_profile=" << hero_profile << "\n";
            out << "parry_delay_ms=" << parry_delay_ms << "\n";
            out << "unblockable_parry_delay_ms=" << unblockable_parry_delay_ms << "\n";
            out << "counter_gb_delay_ms=" << counter_gb_delay_ms << "\n";
            out << "bash_dodge_delay_ms=" << bash_dodge_delay_ms << "\n";
            out << "action_cooldown_ms=" << action_cooldown_ms << "\n";
            out << "random_block_delay_min_ms=" << random_block_delay_min_ms << "\n";
            out << "random_block_delay_max_ms=" << random_block_delay_max_ms << "\n";
            out << "random_dodge_bash_delay_min_ms=" << random_dodge_bash_delay_min_ms << "\n";
            out << "random_dodge_bash_delay_max_ms=" << random_dodge_bash_delay_max_ms << "\n";
            out << "enable_random_delays=" << (enable_random_delays ? 1 : 0) << "\n";
            out << "use_gaussian_distribution=" << (use_gaussian_distribution ? 1 : 0) << "\n";

            out << "[Features]\n";
            out << "auto_block_enabled=" << (auto_block_enabled ? 1 : 0) << "\n";
            out << "gank_autoblock_enabled=" << (gank_autoblock_enabled ? 1 : 0) << "\n";
            out << "auto_lmb_on_gb=" << (auto_lmb_on_gb ? 1 : 0) << "\n";
            out << "auto_rmb_black_prior=" << (auto_rmb_black_prior ? 1 : 0) << "\n";
            out << "auto_parry_all=" << (auto_parry_all ? 1 : 0) << "\n";
            out << "auto_deflect_enabled=" << (auto_deflect_enabled ? 1 : 0) << "\n";
            out << "auto_flip_enabled=" << (auto_flip_enabled ? 1 : 0) << "\n";
            out << "warden_crushing_counter=" << (warden_crushing_counter ? 1 : 0) << "\n";
            out << "kensei_deflect=" << (kensei_deflect ? 1 : 0) << "\n";
            out << "hidden_stance_evasion=" << (hidden_stance_evasion ? 1 : 0) << "\n";
            out << "highlander_stance_dodge=" << (highlander_stance_dodge ? 1 : 0) << "\n";
            out << "shaman_bite_enabled=" << (shaman_bite_enabled ? 1 : 0) << "\n";
            out << "anti_nuxia_trap=" << (anti_nuxia_trap ? 1 : 0) << "\n";
            out << "full_block_mode=" << (full_block_mode ? 1 : 0) << "\n";
            out << "crushing_counter_all=" << (crushing_counter_all ? 1 : 0) << "\n";
            out << "follow_up_dodge_attack=" << (follow_up_dodge_attack ? 1 : 0) << "\n";
            out << "follow_up_deflect_attack=" << (follow_up_deflect_attack ? 1 : 0) << "\n";

            out << "[Keybinds]\n";
            out << "key_top=" << key_top << "\n";
            out << "key_left=" << key_left << "\n";
            out << "key_right=" << key_right << "\n";
            out << "key_dodge=" << key_dodge << "\n";
            out << "key_gb=" << key_gb << "\n";
            out << "key_fullblock=" << key_fullblock << "\n";
        }

        void LoadFromFile(const std::string& path = "") {
            std::string target = path.empty() ? GetConfigFilePath() : path;
            std::ifstream in(target);
            if (!in.is_open()) return;
            std::string line;
            auto parseRect = [](const std::string& val, Rect& r) {
                try {
                    std::stringstream ss(val);
                    char comma;
                    ss >> r.x >> comma >> r.y >> comma >> r.w >> comma >> r.h;
                } catch (...) {}
            };

            while (std::getline(in, line)) {
                if (line.empty() || line[0] == '[' || line[0] == '#') continue;
                size_t eq = line.find('=');
                if (eq == std::string::npos) continue;
                std::string key = line.substr(0, eq);
                std::string val = line.substr(eq + 1);

                try {
                    if (key == "top") parseRect(val, top_zone);
                    else if (key == "left") parseRect(val, left_zone);
                    else if (key == "right") parseRect(val, right_zone);
                    else if (key == "center") parseRect(val, center_zone);
                    else if (key == "gank_left") parseRect(val, gank_left);
                    else if (key == "gank_right") parseRect(val, gank_right);
                    else if (key == "hero_profile") hero_profile = std::stoi(val);
                    else if (key == "parry_delay_ms") parry_delay_ms = std::stoi(val);
                    else if (key == "unblockable_parry_delay_ms") unblockable_parry_delay_ms = std::stoi(val);
                    else if (key == "counter_gb_delay_ms") counter_gb_delay_ms = std::stoi(val);
                    else if (key == "bash_dodge_delay_ms") bash_dodge_delay_ms = std::stoi(val);
                    else if (key == "action_cooldown_ms") action_cooldown_ms = std::stoi(val);
                    else if (key == "random_block_delay_min_ms") random_block_delay_min_ms = std::stoi(val);
                    else if (key == "random_block_delay_max_ms") random_block_delay_max_ms = std::stoi(val);
                    else if (key == "random_dodge_bash_delay_min_ms") random_dodge_bash_delay_min_ms = std::stoi(val);
                    else if (key == "random_dodge_bash_delay_max_ms") random_dodge_bash_delay_max_ms = std::stoi(val);
                    else if (key == "enable_random_delays") enable_random_delays = (std::stoi(val) != 0);
                    else if (key == "use_gaussian_distribution") use_gaussian_distribution = (std::stoi(val) != 0);
                    else if (key == "auto_block_enabled") auto_block_enabled = (std::stoi(val) != 0);
                    else if (key == "gank_autoblock_enabled") gank_autoblock_enabled = (std::stoi(val) != 0);
                    else if (key == "auto_lmb_on_gb") auto_lmb_on_gb = (std::stoi(val) != 0);
                    else if (key == "auto_rmb_black_prior") auto_rmb_black_prior = (std::stoi(val) != 0);
                    else if (key == "auto_parry_all") auto_parry_all = (std::stoi(val) != 0);
                    else if (key == "auto_deflect_enabled") auto_deflect_enabled = (std::stoi(val) != 0);
                    else if (key == "auto_flip_enabled") auto_flip_enabled = (std::stoi(val) != 0);
                    else if (key == "warden_crushing_counter") warden_crushing_counter = (std::stoi(val) != 0);
                    else if (key == "key_top") key_top = static_cast<WORD>(std::stoi(val));
                    else if (key == "key_left") key_left = static_cast<WORD>(std::stoi(val));
                    else if (key == "key_right") key_right = static_cast<WORD>(std::stoi(val));
                    else if (key == "key_dodge") key_dodge = static_cast<WORD>(std::stoi(val));
                    else if (key == "key_gb") key_gb = static_cast<WORD>(std::stoi(val));
                    else if (key == "key_fullblock") key_fullblock = static_cast<WORD>(std::stoi(val));
                } catch (...) {}
            }
        }
    };

}
