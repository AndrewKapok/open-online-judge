@echo off
REM build_cmark_gfm.bat - Build cmark-gfm static library for open-online-judge
REM
REM Usage: build_cmark_gfm.bat <OutputDir> [Configuration] [Platform]
REM   OutputDir   - Output directory for the .lib files (required)
REM   Config      - Debug or Release (default: Release)
REM   Platform    - x64 or Win32 (default: x64)
REM
REM VS Pre-build event example:
REM   call "$(ProjectDir)build_cmark_gfm.bat" "$(OutDir)" $(Configuration) $(Platform)

setlocal enabledelayedexpansion

set OUTPUT_DIR=%~1
if "%OUTPUT_DIR%"=="" (
    echo [cmark-gfm] Error: Output directory is required
    exit /b 1
)

set CONFIG=%~2
if "%CONFIG%"=="" set CONFIG=Release

set PLATFORM=%~3
if "%PLATFORM%"=="" set PLATFORM=x64

echo ============================================
echo  cmark-gfm Static Library Builder
echo ============================================
echo  OutputDir: %OUTPUT_DIR%
echo  Config:    %CONFIG%
echo  Platform:  %PLATFORM%
echo ============================================

REM Determine script directory
set SCRIPT_DIR=%~dp0
set SCRIPT_DIR=%SCRIPT_DIR:~0,-1%

REM Source directories (relative to this script in core/)
set CMARK_SRC_DIR=%SCRIPT_DIR%\cmark-gfm\src
set CMARK_EXT_DIR=%SCRIPT_DIR%\cmark-gfm\extensions
set BUILD_DIR=%SCRIPT_DIR%\cmark-gfm\build\%CONFIG%\%PLATFORM%

REM Output library paths
set CORE_LIB=%OUTPUT_DIR%\cmark-gfm.lib
set EXT_LIB=%OUTPUT_DIR%\cmark-gfm-extensions.lib

REM Create directories
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

REM ============================================
REM Step 1: Generate required headers
REM ============================================

if not exist "%BUILD_DIR%\config.h" (
    echo [cmark-gfm] Generating config.h...
    (
        echo #ifndef CMARK_CONFIG_H
        echo #define CMARK_CONFIG_H
        echo #ifdef __cplusplus
        echo extern "C" {
        echo #endif
        echo #define HAVE_STDBOOL_H 1
        echo #include ^<stdbool.h^>
        echo #ifndef CMARK_INLINE
        echo #if defined(_MSC_VER^) ^&^& !defined(__cplusplus^)
        echo #define CMARK_INLINE __inline
        echo #else
        echo #define CMARK_INLINE inline
        echo #endif
        echo #endif
        echo #ifdef __cplusplus
        echo }
        echo #endif
        echo #endif
    ) > "%BUILD_DIR%\config.h"
)

if not exist "%BUILD_DIR%\cmark-gfm_version.h" (
    echo [cmark-gfm] Generating cmark-gfm_version.h...
    (
        echo #ifndef CMARK_GFM_VERSION_H
        echo #define CMARK_GFM_VERSION_H
        echo #define CMARK_GFM_VERSION ^((0 ^<^< 24^) ^| (29 ^<^< 16^) ^| (0 ^<^< 8^) ^| 13^)
        echo #define CMARK_GFM_VERSION_STRING "0.29.0.gfm.13"
        echo #endif
    ) > "%BUILD_DIR%\cmark-gfm_version.h"
)

if not exist "%BUILD_DIR%\cmark-gfm_export.h" (
    echo [cmark-gfm] Generating cmark-gfm_export.h ^(static build^)...
    (
        echo #ifndef CMARK_GFM_EXPORT_H
        echo #define CMARK_GFM_EXPORT_H
        echo #define CMARK_GFM_EXPORT
        echo #define CMARK_GFM_NO_EXPORT
        echo #endif
    ) > "%BUILD_DIR%\cmark-gfm_export.h"
)

if not exist "%BUILD_DIR%\cmark-gfm-extensions_export.h" (
    echo [cmark-gfm] Generating cmark-gfm-extensions_export.h ^(static build^)...
    (
        echo #ifndef CMARK_GFM_EXTENSIONS_EXPORT_H
        echo #define CMARK_GFM_EXTENSIONS_EXPORT_H
        echo #define CMARK_GFM_EXTENSIONS_EXPORT
        echo #define CMARK_GFM_EXTENSIONS_NO_EXPORT
        echo #endif
    ) > "%BUILD_DIR%\cmark-gfm-extensions_export.h"
)

REM ============================================
REM Step 2: Set up MSVC build environment
REM ============================================

where cl.exe >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    if not "%VCInstallDir%"=="" (
        if /i "%PLATFORM%"=="x64" (
            set VC_ARCH=amd64
        ) else (
            set VC_ARCH=x86
        )
        echo [cmark-gfm] Setting up MSVC environment ^(!VC_ARCH!^)...
        call "%VCInstallDir%Auxiliary\Build\vcvarsall.bat" !VC_ARCH! >nul
        if errorlevel 1 (
            echo [cmark-gfm] Error: Failed to set up MSVC environment
            exit /b 1
        )
    ) else (
        echo [cmark-gfm] Error: cl.exe not found and VCInstallDir is not set.
        echo [cmark-gfm] Run this from a Visual Studio developer prompt, or
        echo [cmark-gfm] configure the pre-build event correctly in project settings.
        exit /b 1
    )
)

REM ============================================
REM Step 3: Set compiler and linker flags
REM ============================================

set COMMON_DEFINES=/D_CRT_SECURE_NO_WARNINGS /DCMARK_GFM_STATIC_DEFINE /DCMARK_GFM_EXTENSIONS_STATIC_DEFINE
set WARN_FLAGS=/W4 /wd4706 /wd4204 /wd4221 /wd4100

if /i "%CONFIG%"=="Debug" (
    set COMPILE_FLAGS=/nologo /Od /MDd /Zi %WARN_FLAGS% %COMMON_DEFINES%
) else (
    set COMPILE_FLAGS=/nologo /O2 /MD %WARN_FLAGS% %COMMON_DEFINES%
)

set CORE_INCLUDES=/I"%CMARK_SRC_DIR%" /I"%BUILD_DIR%"
set EXT_INCLUDES=/I"%CMARK_EXT_DIR%" /I"%CMARK_SRC_DIR%" /I"%BUILD_DIR%"

REM ============================================
REM Step 4: Build core cmark-gfm library
REM ============================================

echo [cmark-gfm] Compiling core library sources...

set CORE_OBJS=
for %%F in (
    cmark node iterator blocks inlines scanners
    utf8 buffer references footnotes map render
    man xml html commonmark plaintext latex
    houdini_href_e houdini_html_e houdini_html_u
    cmark_ctype arena linked_list syntax_extension
    registry plugin
) do (
    set OBJ_FILE=%BUILD_DIR%\%%F.obj
    set SRC_FILE=%CMARK_SRC_DIR%\%%F.c
    echo [cmark-gfm]   cl.exe %%F.c
    cl.exe %COMPILE_FLAGS% %CORE_INCLUDES% /Fo"!OBJ_FILE!" /c "!SRC_FILE!"
    if errorlevel 1 (
        echo [cmark-gfm] Error: Failed to compile %%F.c
        exit /b 1
    )
    set "CORE_OBJS=!CORE_OBJS! "!OBJ_FILE!""
)

echo [cmark-gfm] Linking core library...
lib.exe /nologo /out:"%CORE_LIB%" %CORE_OBJS%
if errorlevel 1 (
    echo [cmark-gfm] Error: Failed to link core library
    exit /b 1
)
echo [cmark-gfm] Core library created: %CORE_LIB%

REM ============================================
REM Step 5: Build extensions library
REM ============================================

echo [cmark-gfm] Compiling extensions sources...

set EXT_OBJS=
for %%F in (
    core-extensions table strikethrough autolink
    tagfilter ext_scanners tasklist
) do (
    set OBJ_FILE=%BUILD_DIR%\%%F.obj
    set SRC_FILE=%CMARK_EXT_DIR%\%%F.c
    echo [cmark-gfm]   cl.exe %%F.c
    cl.exe %COMPILE_FLAGS% %EXT_INCLUDES% /Fo"!OBJ_FILE!" /c "!SRC_FILE!"
    if errorlevel 1 (
        echo [cmark-gfm] Error: Failed to compile %%F.c
        exit /b 1
    )
    set "EXT_OBJS=!EXT_OBJS! "!OBJ_FILE!""
)

echo [cmark-gfm] Linking extensions library...
lib.exe /nologo /out:"%EXT_LIB%" %EXT_OBJS%
if errorlevel 1 (
    echo [cmark-gfm] Error: Failed to link extensions library
    exit /b 1
)
echo [cmark-gfm] Extensions library created: %EXT_LIB%

echo ============================================
echo  cmark-gfm Build Complete!
echo  Libraries:
echo    %CORE_LIB%
echo    %EXT_LIB%
echo ============================================
exit /b 0