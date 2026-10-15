#pragma once
#include <cstdint>
#include "config.hpp"
#include "capture.hpp"

namespace inkwell {

    enum class ThreatState {
        None,
        RedAttack,
        UnblockableAttack,
        ParryFlash,
        OrangeBash,
        RedShieldGB,
        NuxiaTrap,
        ShamanBleed,
        GankLeftThreat,
        GankRightThreat
    };

    struct DetectionResult {
        ThreatState state = ThreatState::None;
        float ratio = 0.0f;
        float red_ratio = 0.0f;
        float orange_ratio = 0.0f;
        float flash_ratio = 0.0f;
        float trap_ratio = 0.0f;
        float bleed_ratio = 0.0f;
        int red_matches = 0;
        int orange_matches = 0;
        int flash_matches = 0;
        int shield_matches = 0;
        int trap_matches = 0;
        int bleed_matches = 0;
        int peak_r = 0;
        int peak_g = 0;
        int peak_b = 0;
    };

    class ColorDetector {
    public:
        static DetectionResult AnalyzeDirectional(const uint32_t* pixels, int width, int height, int stride,
                                                 const ColorFilter& red_filter,
                                                 const ColorFilter& flash_filter,
                                                 bool zoneCurrentlyActive = false);

        static DetectionResult AnalyzeCenter(const uint32_t* pixels, int width, int height, int stride,
                                             const ColorFilter& shield_filter,
                                             const ColorFilter& orange_filter,
                                             const ColorFilter& trap_filter,
                                             const ColorFilter& bleed_filter,
                                             bool currentlyActive = false);

        static DetectionResult AnalyzeGankPeriphery(const uint32_t* pixels, int width, int height, int stride,
                                                    const ColorFilter& red_filter);

        static DetectionResult AnalyzeSubViewDirectional(const SubView& sv, const ColorFilter& red_filter, const ColorFilter& flash_filter, bool zoneActive) {
            return AnalyzeDirectional(sv.pixels, sv.width, sv.height, sv.stride, red_filter, flash_filter, zoneActive);
        }

        static DetectionResult AnalyzeSubViewCenter(const SubView& sv, const ColorFilter& shield_filter, const ColorFilter& orange_filter, const ColorFilter& trap_filter, const ColorFilter& bleed_filter, bool active) {
            return AnalyzeCenter(sv.pixels, sv.width, sv.height, sv.stride, shield_filter, orange_filter, trap_filter, bleed_filter, active);
        }

        static DetectionResult AnalyzeSubViewGank(const SubView& sv, const ColorFilter& red_filter) {
            return AnalyzeGankPeriphery(sv.pixels, sv.width, sv.height, sv.stride, red_filter);
        }
    };

}
