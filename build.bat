@echo off
setlocal enabledelayedexpansion

cd /d "%~dp0"

echo ============================================================
echo   Building Inkwell For Honor Script (MSVC x64 C++17)
echo ============================================================

:: 1. Check if cl.exe is already active in current environment
where cl.exe >nul 2>&1
if %ERRORLEVEL% equ 0 (
    echo [+] Using active MSVC compiler from PATH...
    goto :COMPILE
)

:: 2. Auto-discover Visual Studio via vswhere (Standard across all modern Windows PCs)
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist "%VSWHERE%" (
    for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
        set "VS_INSTALL=%%i"
    )
    if defined VS_INSTALL (
        if exist "!VS_INSTALL!\VC\Auxiliary\Build\vcvars64.bat" (
            echo [+] Auto-detected Visual Studio at: !VS_INSTALL!
            call "!VS_INSTALL!\VC\Auxiliary\Build\vcvars64.bat" >nul
            goto :COMPILE
        )
    )
)

:: 3. Probing fallback paths across standard editions and versions
set "PROBED="
for %%P in (
    "%ProgramFiles%\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
    "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
    "%ProgramFiles%\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
    "%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
    "%ProgramFiles%\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
    "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
    "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat"
    "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
    "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
) do (
    if exist "%%~P" (
        echo [+] Found MSVC build environment: %%~P
        call "%%~P" >nul
        goto :COMPILE
    )
)

echo [-] Error: Microsoft Visual C++ Build Tools or Visual Studio not found on this system.
echo [-] Please install Visual Studio with "Desktop development with C++" workload.
pause
exit /b 1

:COMPILE
echo [+] Compiling source files into "4h menu inkwell.exe"...
cl.exe /nologo /O2 /MT /EHsc /utf-8 /std:c++17 ^
    main.cpp capture.cpp detector.cpp input.cpp overlay.cpp ^
    /Fe:"4h menu inkwell.exe" ^
    user32.lib gdi32.lib kernel32.lib winmm.lib ^
    /link /SUBSYSTEM:WINDOWS /ENTRY:WinMainCRTStartup /MANIFESTUAC:"level='requireAdministrator' uiAccess='false'"

if %ERRORLEVEL% equ 0 (
    echo.
    echo ============================================================
    echo [+] BUILD SUCCESSFUL: "4h menu inkwell.exe" generated!
    echo ============================================================
    del /f /q *.obj >nul 2>&1
    echo [+] Ready to run: "4h menu inkwell.exe"
) else (
    echo.
    echo [-] BUILD FAILED! Check error diagnostics above.
    pause
    exit /b 1
)
