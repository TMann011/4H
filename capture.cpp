#include "capture.hpp"
#include <iostream>
#include <algorithm>

namespace inkwell {

    ScreenCapture::ScreenCapture() = default;

    ScreenCapture::~ScreenCapture() {
        Cleanup();
    }

    bool ScreenCapture::Initialize(int width, int height) {
        if (m_hdcMem && width <= m_allocWidth && height <= m_allocHeight) {
            m_width = width;
            m_height = height;
            return true;
        }

        Cleanup();

        m_width = width;
        m_height = height;
        m_allocWidth = (std::max)(width, 600);
        m_allocHeight = (std::max)(height, 400);

        m_hdcScreen = GetDC(nullptr);
        if (!m_hdcScreen) return false;

        m_hdcMem = CreateCompatibleDC(m_hdcScreen);
        if (!m_hdcMem) {
            ReleaseDC(nullptr, m_hdcScreen);
            m_hdcScreen = nullptr;
            return false;
        }

        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = m_allocWidth;
        bmi.bmiHeader.biHeight = -m_allocHeight; // Top-down DIB
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        void* bits = nullptr;
        m_hBitmap = CreateDIBSection(m_hdcMem, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
        if (!m_hBitmap || !bits) {
            Cleanup();
            return false;
        }

        m_pixels = static_cast<uint32_t*>(bits);
        m_hOldBmp = SelectObject(m_hdcMem, m_hBitmap);

        return true;
    }

    void ScreenCapture::Cleanup() {
        if (m_hdcMem) {
            if (m_hOldBmp) SelectObject(m_hdcMem, m_hOldBmp);
            DeleteDC(m_hdcMem);
            m_hdcMem = nullptr;
        }
        if (m_hBitmap) {
            DeleteObject(m_hBitmap);
            m_hBitmap = nullptr;
            m_pixels = nullptr;
        }
        if (m_hdcScreen) {
            ReleaseDC(nullptr, m_hdcScreen);
            m_hdcScreen = nullptr;
        }
        m_width = 0;
        m_height = 0;
        m_allocWidth = 0;
        m_allocHeight = 0;
    }

    bool ScreenCapture::CaptureFullCombatRegion(int scanX, int scanY, int width, int height) {
        if (width <= 0 || height <= 0) return false;
        if (!Initialize(width, height)) return false;

        m_scanX = scanX;
        m_scanY = scanY;

        BOOL res = BitBlt(m_hdcMem, 0, 0, width, height, m_hdcScreen, scanX, scanY, SRCCOPY);
        return res != FALSE;
    }

    SubView ScreenCapture::GetSubView(int subX, int subY, int subW, int subH) const {
        SubView sv = {};
        if (!m_pixels || subW <= 0 || subH <= 0) return sv;

        int offsetX = subX - m_scanX;
        int offsetY = subY - m_scanY;

        if (offsetX < 0 || offsetY < 0 || offsetX + subW > m_allocWidth || offsetY + subH > m_allocHeight) {
            return sv;
        }

        sv.pixels = m_pixels + (offsetY * m_allocWidth) + offsetX;
        sv.width  = subW;
        sv.height = subH;
        sv.stride = m_allocWidth;
        return sv;
    }

    bool ScreenCapture::Capture(int x, int y, int width, int height) {
        return CaptureFullCombatRegion(x, y, width, height);
    }

}
