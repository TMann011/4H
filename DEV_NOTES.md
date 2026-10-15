# Inkwell For Honor Script - Pro Feature Console & Anti-Cheat Reference
**Version**: 3.0.0-ENTERPRISE  
**Core Architect**: `cook45` Core & `clack` Systems Engineering  
**Target Environment**: Windows 10/11 x64, Win32 GDI / DirectInput Hardware Synthesis  
**Output Binary**: `4h menu inkwell.exe` (Subsystem: Windows, Manifest: RequireAdministrator)

---

## 1. Technical Synopsis & Architecture

This software provides a sub-millisecond external reaction console built in C++17. It couples Desktop GDI memory mapping (`BitBlt` / `CreateDIBSection`), chromatic hysteresis filters, Gaussian reaction humanization (`std::normal_distribution`), DirectInput hardware scancode injection, and Win32 layered overlay transparent drawing (`WS_EX_LAYERED` / `WDA_EXCLUDEFROMCAPTURE`).

---

## 2. Multi-Tab Feature Console Architecture

### Tab 0: Core Combat
- **AutoBlock**: Independent directional stance matching on top, left, and right attack chevrons.
- **Gank AutoBlock**: Periphery indicator scanning (`gank_left`, `gank_right`) for external off-target threats.
- **Auto Parry All**: Parry timing engine with feint-proof white flash burst validation.
- **Strict Feint-Proof Filter**: Rejects early RMB commitments unless confirmed parry flash flare is active.
- **Parry Unblockables Only**: Pure block on standard attacks, auto-parry on unblockable flame chevrons.
- **Directional Mouse Flick**: Directional mouse offset movement for stance switching.

### Tab 1: Hero Specialists
- **Warden Crushing Counter**: Top Light Attack (`K` + `LMB`) on top attacks; side parry (`RMB`); parry unblockables.
- **Kensei Superior Block**: Forward Dodge (`W` + `Space`) on top attacks; side parry; parry unblockables.
- **Black Prior Bulwark Flip**: Automated Bulwark Counter (`C` Stance + `RMB`) on incoming bashes and attacks.
- **Nobushi / JJ / Highlander Hidden Stance**: Evasive stance input (`C` stance) on incoming bashes/attacks.
- **Highlander Offensive Stance Auto-Dodge**: Automated side dodge in stance mode.
- **Shaman Bleed Bite**: Predator's Mercy bite (`W` + `Space` + `X`) triggered upon magenta bleed status detection.
- **Anti-Nuxia Trap**: Light attack in opposite stance direction or parry override on yellow/orange trap chevron detection.
- **Full Block Mode**: Automated `C` holding for Warlord, Aramusha, Varangian, and BP with parry override on unblockables.
- **Universal Crushing Counter**: Directional Light Attack (`LMB`) during startup window for all CC heroes.

### Tab 2: Punishes & Evasion
- **Auto LMB on GB**: Instant Light Attack (`LMB`) counter upon center `RedShieldGB` detection (also active in Full Block).
- **Auto RMB BP Flip on GB**: Automated Bulwark Flip (`RMB` in Full Block) when opponent attempts GB.
- **Auto Counter-Guardbreak**: Multi-pulse CGB burst (`X` / `MMB` / `Mouse5`).
- **Auto Dodge Bash**: Side dodge (`A`/`D` + `Space`) upon `OrangeBash` detection.
- **Follow-up Dodge / Deflect Attacks**: Executed follow-up attack (`LMB`/`RMB`) after completing dodge or deflect windows.

### Tab 3: Timing & Humanization Engine
- **Non-Deterministic Delay Jitter**: Frame delay offset randomized via `std::mt19937`.
- **Gaussian Reaction Distribution**: Mimics authentic human reaction curves (`std::normal_distribution` with configurable standard deviation $\sigma = 8.5\,\text{ms}$).
- **Hero Roster Profiles**: Presets for Assassin (Fast Lights ~400ms), Hybrid, Heavy / Charged, Vanguard, and Outlander / Special.

### Tab 4: Custom Keybind Manager
- Rebind Virtual Key codes (`key_top`, `key_left`, `key_right`, `key_dodge`, `key_gb`, `key_fullblock`) dynamically via live GUI clicks and keyboard capture.

---

## 3. Anti-Cheat (EAC) Technical Analysis & Mitigation Mechanics

### 3.1. EAC Detection Vectors & System Countermeasures

1. **User-Mode Screen Capture Hooks**:
   - *Detection Vector*: Anti-cheats inspect open handles and capture APIs (`BitBlt`, `GetPixel`, `DXGI Desktop Duplication`) to detect visual overlays.
   - *Mitigation*: `SetWindowDisplayAffinity(m_hwnd, WDA_EXCLUDEFROMCAPTURE)` removes the overlay from OS desktop duplication capture streams, rendering it 100% invisible to anti-cheat screenshot routines.

2. **Synthetic Input Flags (`NtUserSendInput`)**:
   - *Detection Vector*: User-mode `SendInput` API calls generate low-level synthetic event flags (`LLKHF_INJECTED` / `LLMHF_INJECTED`) in Windows `KUSER_SHARED_DATA` and input queues.
   - *Mitigation*: Inputs use DirectInput hardware scancodes (`KEYEVENTF_SCANCODE`). In kernel-restricted environments, user-mode software can be paired with kernel input drivers or driver emulators (such as Kevlar or KernEmul) to synthesize physical USB HID reports directly into `kbdclass.sys` / `mouclass.sys`.

3. **Behavioral Reaction Time Analysis**:
   - *Detection Vector*: Static response times (e.g. exactly 150ms every reaction) trigger statistical anomaly detection algorithms.
   - *Mitigation*: The Gaussian Reaction Distribution Engine introduces humanized variance centered around natural human reaction curves.

---

## 4. Verification & Build Diagnostics

- **Compiler**: MSVC x64 C++17 (`cl.exe`)
- **Build Script**: `build.bat`
- **Binary**: `4h menu inkwell.exe`
