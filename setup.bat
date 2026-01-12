@echo off
setlocal enabledelayedexpansion

:: enen setup script for Windows
:: Downloads and installs IntgrNN dependency

echo enen setup
echo ==========
echo.

set "SCRIPT_DIR=%~dp0"
set "EXTERNAL_DIR=%SCRIPT_DIR%external"
set "INTGRNN_DIR=%EXTERNAL_DIR%\intgr_nn"
set "INTGRNN_REPO=double-star-games/intgr_nn"
set "PLATFORM=windows-x64"

:: Check if already installed
if exist "%INTGRNN_DIR%\include" (
    if exist "%INTGRNN_DIR%\lib\intgr_nn.lib" (
        echo IntgrNN already installed at %INTGRNN_DIR%
        echo Remove external\intgr_nn to reinstall.
        exit /b 0
    )
)

:: Check for GitHub CLI
where gh >nul 2>nul
if %errorlevel% neq 0 (
    echo ERROR: GitHub CLI ^(gh^) not found.
    echo.
    echo Please install it from: https://cli.github.com/
    echo Or download IntgrNN manually from:
    echo   https://github.com/double-star-games/intgr_nn/releases
    exit /b 1
)

echo Detected platform: %PLATFORM%
echo.

:: Create external directory
if not exist "%EXTERNAL_DIR%" mkdir "%EXTERNAL_DIR%"

:: Get latest release tag
echo Fetching IntgrNN...
for /f "tokens=*" %%i in ('gh release view --repo %INTGRNN_REPO% --json tagName -q .tagName 2^>nul') do set "RELEASE_TAG=%%i"

if "%RELEASE_TAG%"=="" (
    echo ERROR: Could not find IntgrNN release.
    echo.
    echo Please download manually from:
    echo   https://github.com/double-star-games/intgr_nn/releases
    exit /b 1
)

echo Downloading IntgrNN %RELEASE_TAG% for %PLATFORM%...

:: Create temp directory
set "TEMP_DIR=%TEMP%\intgr_nn_setup_%RANDOM%"
mkdir "%TEMP_DIR%"

:: Download release
set "PATTERN=intgr_nn-%PLATFORM%.zip"
gh release download "%RELEASE_TAG%" --repo %INTGRNN_REPO% --pattern "%PATTERN%" --dir "%TEMP_DIR%" 2>nul

if not exist "%TEMP_DIR%\%PATTERN%" (
    echo ERROR: Failed to download %PATTERN%
    rmdir /s /q "%TEMP_DIR%" 2>nul
    exit /b 1
)

:: Extract
echo Extracting...
if not exist "%INTGRNN_DIR%" mkdir "%INTGRNN_DIR%"
powershell -Command "Expand-Archive -Path '%TEMP_DIR%\%PATTERN%' -DestinationPath '%INTGRNN_DIR%' -Force"

:: Cleanup
rmdir /s /q "%TEMP_DIR%" 2>nul

:: Verify
if exist "%INTGRNN_DIR%\include" (
    echo IntgrNN installed successfully.
    exit /b 0
) else (
    echo ERROR: Extraction failed.
    exit /b 1
)
