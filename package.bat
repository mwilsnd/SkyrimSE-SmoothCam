@echo off
chcp 65001 > NUL
if "%SKYRIM_DIR%"=="" set "SKYRIM_DIR=C:/git/SkyrimTools"
set "PAPYRUS_COMPILER=%SKYRIM_DIR%/Papyrus Compiler/PapyrusCompiler.exe"
set "SCRIPTS_FOLDER=%SKYRIM_DIR%/Data/Source/Scripts"

echo 📣 ^| This might take a while! Destination: ./SmoothCam.zip

echo 🧪 ^| Testing for working papyrus compiler...
if not exist "%PAPYRUS_COMPILER%" (
    echo 🛑 ^| Unable to find papyrus compiler, aborting
    exit /b 1
)

echo 🧪 ^| Testing for 7-zip...
7z.exe 1>nul
if not ERRORLEVEL 0 (
    echo 🛑 ^| Unable to find 7-zip, aborting
    exit /b
)

echo 🧪 ^| Testing for working D compiler (ldc2)...
ldc2.exe 1>nul 2>nul
if not ERRORLEVEL 0 (
    echo 🛑 ^| Unable to find LDC, aborting
    exit /b
)

if not "%1" == "OnlyScripts" (
    echo 🧹 ^| Cleaning up old build artifacts...
    rmdir Release_Package /s /q
    buck clean
)

echo 📜 ^| Building paper
buck build :paper.exe

if not "%1" == "OnlyScripts" (
    echo 🎥 ^| Building SmoothCam AE
    buck build :SmoothCamAERelease#windows-x86_64
    echo 🎥 ^| Building SmoothCam AE,Pre629
    buck build :SmoothCamAEPre629Release#windows-x86_64
    echo 🎥 ^| Building SmoothCam SSE
    buck build :SmoothCamSSERelease#windows-x86_64

    if not ERRORLEVEL 0 (
        echo 🛑 ^| Build failure, aborting
        exit /b
    )
)

echo 📦 ^| Creating new release package...
xcopy "./Package" "./Release_Package" /s /e /q /i /y > NUL

echo 📜 ^| Invoking paper...
"buck-out/gen/paper.exe#binary/paper.exe" "CodeGen/MCM/mcm/mcm.psc" "../../../Release_Package/00 Data/SmoothCamMCM.psc"

echo 🚽 ^| Compiling generated papyrus...
echo f | xcopy /f /y "Release_Package/00 Data/SmoothCamMCM.psc" "%SCRIPTS_FOLDER%/SmoothCamMCM.psc" > NUL
"%PAPYRUS_COMPILER%" "SmoothCamMCM.psc" -f="%SCRIPTS_FOLDER%/TESV_Papyrus_Flags.flg" -i="%SCRIPTS_FOLDER%" -o="Release_Package/00 Data"

echo 🔎 ^| Copying artifacts to package...
echo f | xcopy /f /y "buck-out/gen/SmoothCamAEModuleRelease#shared,windows-x86_64/SmoothCamAE.dll" "Release_Package/00 Data/AE/SmoothCam.dll" > NUL
echo f | xcopy /f /y "buck-out/gen/SmoothCamAEPre629ModuleRelease#shared,windows-x86_64/SmoothCamAEPre629.dll" "Release_Package/00 Data/AE-Pre629/SmoothCam.dll" > NUL
echo f | xcopy /f /y "buck-out/gen/SmoothCamSSEModuleRelease#shared,windows-x86_64/SmoothCamSSE.dll" "Release_Package/00 Data/SSE/SmoothCam.dll" > NUL

if not "%1" == "OnlyScripts" (
    del "Release_Package\00 Data\AE\placeholder" /f /q
    del "Release_Package\00 Data\AE-Pre629\placeholder" /f /q
    del "Release_Package\00 Data\SSE\placeholder" /f /q
    del "Release_Package\00 Data\SmoothCamMCM.psc" /f /q
)

echo 📚 ^| Compressing package...
7z a -tzip "SmoothCam.zip" "Release_Package" 1>nul

echo 🦀🔥🦀 Packaging completed 🦀🔥🦀