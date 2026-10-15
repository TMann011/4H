# Inkwell For Honor - External Reaction & Defense System

High-throughput, sub-millisecond reaction console built in C++17 for Windows x64. Couples Win32 Desktop GDI memory mapping (`BitBlt`), chromatic hysteresis filters, Gaussian reaction humanization, DirectInput scancode injection, and anti-screenshot transparent overlay drawing (`WDA_EXCLUDEFROMCAPTURE`).

---

## Features

### Core Combat
- **AutoBlock**: Directional stance matching across top, left, and right attack chevrons.
- **Auto Parry All**: White parry flash burst validation with feint-proof filtering.
- **Parry Unblockables Only**: Standard block on regular attacks, automated parry on unblockables.
- **Gank AutoBlock**: Periphery indicator scanning for multi-target threat containment.
- **Anti-Nuxia Trap & Shaman Bite**: Dynamic counters for hero-specific unblockable states.
- **Auto Counter-Guardbreak (CGB)**: Sub-frame reaction burst to broken shield indicators.
- **Hero Stance Specialists**: Profiles for Warden Crushing Counter, Kensei Superior Block, Black Prior Bulwark Flip, and Hidden Stance evasion.

### Anti-Cheat & Humanization
- **Screen-Capture Invisibility**: `SetWindowDisplayAffinity(WDA_EXCLUDEFROMCAPTURE)` removes the overlay from OBS, Discord, and anti-cheat desktop duplication capture streams.
- **DirectInput Hardware Scancodes**: Low-level `SendInput` utilizing raw hardware scancodes.
- **Gaussian Jitter Engine**: Randomizes reaction timing with authentic human variance curves ($\sigma = 8.5\,\text{ms}$) to defeat statistical detection.

---

## Hotkeys

| Hotkey | Action |
| :--- | :--- |
| **F1** | Toggle Automation Active / Paused |
| **F2** | Toggle HUD / Calibration Box Overlays |
| **F3** | Auto-Calibrate Combat Detection Boxes to Screen Stance |
| **F11** | Toggle Interactive Configuration Menu |

---

## Build Instructions

### Requirements
- Windows 10 / 11 (64-bit)
- Visual Studio 2019 / 2022 / v18 or Visual C++ Build Tools with C++17 support

### Compilation
Double-click `build.bat` or run:
```cmd
build.bat
```
The script auto-detects your MSVC installation via `vswhere` or standard compiler paths and outputs `4h menu inkwell.exe`.
