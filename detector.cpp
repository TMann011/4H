#include "detector.hpp"
#include <algorithm>
#include <cmath>

namespace inkwell {

    DetectionResult ColorDetector::AnalyzeDirectional(const uint32_t* pixels, int width, int height, int stride,
                                                      const ColorFilter& red_filter,
                                                      const ColorFilter& flash_filter,
                                                      bool zoneCurrentlyActive) {
        DetectionResult result = {};
        if (!pixels || width <= 0 || height <= 0) return result;

        int pitchStride = (stride > 0) ? stride : width;
        int total_pixels = width * height;
        int red_matches = 0;
        int orange_matches = 0;
        int flash_matches = 0;

        int max_r = 0, max_g = 0, max_b = 0;
        int min_x = 9999, max_x = -1, min_y = 9999, max_y = -1;

        for (int y = 0; y < height; ++y) {
            const uint32_t* row = pixels + (y * pitchStride);
            for (int x = 0; x < width; ++x) {
                uint32_t color = row[x];
                uint8_t b = color & 0xFF;
                uint8_t g = (color >> 8) & 0xFF;
                uint8_t r = (color >> 16) & 0xFF;

                if (r > max_r) max_r = r;
                if (g > max_g) max_g = g;
                if (b > max_b) max_b = b;

                // 1. Parry Flash (High-Contrast White Flare Burst - Calibrated for all graphic settings & HDR)
                if (r >= 205 && g >= 195 && b >= 170 &&
                    std::abs(static_cast<int>(r) - static_cast<int>(g)) <= 40 &&
                    std::abs(static_cast<int>(r) - static_cast<int>(b)) <= 50) {
                    flash_matches++;
                }
                // 2. Unblockable Flame Aura
                else if (r >= 195 && g >= 65 && g <= 185 && b <= 95 && (r - g >= 25)) {
                    orange_matches++;
                }
                // 3. Neon Red Directional Attack Chevron
                else if (r >= red_filter.min_r && r <= red_filter.max_r &&
                         g >= red_filter.min_g && g <= red_filter.max_g &&
                         b >= red_filter.min_b && b <= red_filter.max_b &&
                         (r - g >= red_filter.min_contrast) &&
                         (r - b >= red_filter.min_contrast) &&
                         (r >= static_cast<int>(g * 2.5f)) &&
                         (r >= static_cast<int>(b * 2.5f))) {
                    red_matches++;
                    if (x < min_x) min_x = x;
                    if (x > max_x) max_x = x;
                    if (y < min_y) min_y = y;
                    if (y > max_y) max_y = y;
                }
            }
        }

        result.peak_r = max_r;
        result.peak_g = max_g;
        result.peak_b = max_b;

        result.flash_ratio  = static_cast<float>(flash_matches) / total_pixels;
        result.orange_ratio = static_cast<float>(orange_matches) / total_pixels;
        result.red_ratio    = static_cast<float>(red_matches) / total_pixels;

        result.flash_matches  = flash_matches;
        result.orange_matches = orange_matches;
        result.red_matches    = red_matches;

        int cluster_w = (max_x >= min_x) ? (max_x - min_x + 1) : 0;
        int cluster_h = (max_y >= min_y) ? (max_y - min_y + 1) : 0;
        bool validChevronGeometry = ((cluster_w >= 35 && cluster_h >= 22) || (cluster_w >= 22 && cluster_h >= 35) || zoneCurrentlyActive);
        bool notUIBannerWash = (result.red_ratio <= 0.30f && result.flash_ratio <= 0.45f);

        bool hasAttackIndicator = ((red_matches + flash_matches >= 300 && validChevronGeometry) || zoneCurrentlyActive);
        // Calibrated parry flash burst flare threshold (flash_matches >= 70)
        bool hasFlashBurst = (flash_matches >= 70);

        if (hasFlashBurst && hasAttackIndicator && notUIBannerWash) {
            result.state = ThreatState::ParryFlash;
            result.ratio = result.flash_ratio;
            return result;
        }

        if (hasAttackIndicator && result.orange_ratio >= 0.020f && notUIBannerWash) {
            result.state = ThreatState::UnblockableAttack;
            result.ratio = result.orange_ratio;
            return result;
        }

        if (red_matches >= 350 && result.red_ratio >= red_filter.min_ratio && validChevronGeometry && notUIBannerWash) {
            result.state = ThreatState::RedAttack;
            result.ratio = result.red_ratio;
            return result;
        }

        return result;
    }

    DetectionResult ColorDetector::AnalyzeCenter(const uint32_t* pixels, int width, int height, int stride,
                                                 const ColorFilter& shield_filter,
                                                 const ColorFilter& orange_filter,
                                                 const ColorFilter& trap_filter,
                                                 const ColorFilter& bleed_filter,
                                                 bool currentlyActive) {
        DetectionResult result = {};
        if (!pixels || width <= 0 || height <= 0) return result;

        int pitchStride = (stride > 0) ? stride : width;
        int total_pixels = width * height;
        int orange_matches = 0;
        int shield_matches = 0;
        int trap_matches = 0;
        int bleed_matches = 0;

        int max_r = 0, max_g = 0, max_b = 0;

        for (int y = 0; y < height; ++y) {
            const uint32_t* row = pixels + (y * pitchStride);
            for (int x = 0; x < width; ++x) {
                uint32_t color = row[x];
                uint8_t b = color & 0xFF;
                uint8_t g = (color >> 8) & 0xFF;
                uint8_t r = (color >> 16) & 0xFF;

                if (r > max_r) max_r = r;
                if (g > max_g) max_g = g;
                if (b > max_b) max_b = b;

                if (r >= trap_filter.min_r && g >= trap_filter.min_g && g <= trap_filter.max_g && b <= trap_filter.max_b && (r - b >= trap_filter.min_contrast)) {
                    trap_matches++;
                }
                else if (r >= bleed_filter.min_r && b >= bleed_filter.min_b && g <= bleed_filter.max_g) {
                    bleed_matches++;
                }
                else if (r >= shield_filter.min_r && r <= shield_filter.max_r &&
                    g >= shield_filter.min_g && g <= shield_filter.max_g &&
                    b >= shield_filter.min_b && b <= shield_filter.max_b &&
                    (r - g >= shield_filter.min_contrast) &&
                    (r - b >= shield_filter.min_contrast) &&
                    (r >= static_cast<int>(g * 2.4f)) &&
                    (r >= static_cast<int>(b * 2.4f))) {
                    shield_matches++;
                }
                else if (r >= orange_filter.min_r && r <= orange_filter.max_r &&
                         g >= orange_filter.min_g && g <= orange_filter.max_g &&
                         b >= orange_filter.min_b && b <= orange_filter.max_b &&
                         (r - b >= orange_filter.min_contrast)) {
                    orange_matches++;
                }
            }
        }

        result.peak_r = max_r;
        result.peak_g = max_g;
        result.peak_b = max_b;

        result.shield_matches = shield_matches;
        result.orange_matches = orange_matches;
        result.trap_matches   = trap_matches;
        result.bleed_matches  = bleed_matches;

        result.ratio        = static_cast<float>(shield_matches) / total_pixels;
        result.orange_ratio = static_cast<float>(orange_matches) / total_pixels;
        result.trap_ratio   = static_cast<float>(trap_matches) / total_pixels;
        result.bleed_ratio  = static_cast<float>(bleed_matches) / total_pixels;

        if (trap_matches >= 300 || result.trap_ratio >= trap_filter.min_ratio) {
            result.state = ThreatState::NuxiaTrap;
            return result;
        }

        if (bleed_matches >= 200 || result.bleed_ratio >= bleed_filter.min_ratio) {
            result.state = ThreatState::ShamanBleed;
            return result;
        }

        if (shield_matches >= 800 || result.ratio >= shield_filter.min_ratio) {
            result.state = ThreatState::RedShieldGB;
            return result;
        }

        if (orange_matches >= 550 || result.orange_ratio >= orange_filter.min_ratio) {
            result.state = ThreatState::OrangeBash;
            return result;
        }

        return result;
    }

    DetectionResult ColorDetector::AnalyzeGankPeriphery(const uint32_t* pixels, int width, int height, int stride,
                                                         const ColorFilter& red_filter) {
        DetectionResult result = {};
        if (!pixels || width <= 0 || height <= 0) return result;

        int pitchStride = (stride > 0) ? stride : width;
        int total_pixels = width * height;
        int red_matches = 0;

        for (int y = 0; y < height; ++y) {
            const uint32_t* row = pixels + (y * pitchStride);
            for (int x = 0; x < width; ++x) {
                uint32_t color = row[x];
                uint8_t b = color & 0xFF;
                uint8_t g = (color >> 8) & 0xFF;
                uint8_t r = (color >> 16) & 0xFF;

                if (r >= red_filter.min_r && r <= red_filter.max_r &&
                    g >= red_filter.min_g && g <= red_filter.max_g &&
                    b >= red_filter.min_b && b <= red_filter.max_b &&
                    (r - g >= red_filter.min_contrast) &&
                    (r - b >= red_filter.min_contrast)) {
                    red_matches++;
                }
            }
        }

        result.red_matches = red_matches;
        result.red_ratio   = static_cast<float>(red_matches) / total_pixels;

        if (red_matches >= 250 && result.red_ratio >= red_filter.min_ratio) {
            result.state = ThreatState::RedAttack;
        }

        return result;
    }

}
