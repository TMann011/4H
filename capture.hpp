#pragma once
#include <windows.h>
#include <cstdint>
#include "config.hpp"

namespace inkwell {

    struct SubView {
        const uint32_t* pixels = nullptr;
        int width = 0;
        int height = 0;
        int stride = 0;
    };

    class ScreenCapture {
    public:
        ScreenCapture();
        ~ScreenCapture();

        bool Initialize(int width, int height);
        void Cleanup();

        // High-Performance Single-Pass Capture of full combat region
        bool CaptureFullCombatRegion(int scanX, int scanY, int width, int height);
        
        // Fast Sub-View Extraction (O(1) Memory Offset calculation, zero BitBlt overhead)
        SubView GetSubView(int subX, int subY, int subW, int subH) const;

        bool Capture(int x, int y, int width, int height);

        const uint32_t* GetPixels() const { return m_pixels; }
        int GetWidth() const { return m_width; }
        int GetHeight() const { return m_height; }
        int GetAllocWidth() const { return m_allocWidth; }

    private:
        HDC m_hdcScreen = nullptr;
        HDC m_hdcMem = nullptr;
        HBITMAP m_hBitmap = nullptr;
        HGDIOBJ m_hOldBmp = nullptr;
        uint32_t* m_pixels = nullptr;
        int m_width = 0;
        int m_height = 0;
        int m_allocWidth = 0;
        int m_allocHeight = 0;
        int m_scanX = 0;
        int m_scanY = 0;
    };

}
