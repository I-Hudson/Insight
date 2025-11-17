@echo off
@setlocal enabledelayedexpansion

set vendorPath=%~dp0..\..\vendor
set currentDirectory=%~dp0

:: Download pix
call :DOWNLOAD_AND_UNZIP https://www.nuget.org/api/v2/package/WinPixEventRuntime/1.0.220810001                                                                              %vendorPath%\winpixeventruntime
call :DOWNLOAD_AND_UNZIP https://www.nuget.org/api/v2/package/Microsoft.Direct3D.D3D12/1.717.1-preview                                                                      %vendorPath%\Microsoft.Direct3D.D3D12
call :DOWNLOAD_AND_UNZIP https://www.nuget.org/api/v2/package/Microsoft.VCRTForwarders.140/1.0.7                                                                            %vendorPath%\Microsoft.VCRTForwarders.140
call :DOWNLOAD_AND_UNZIP https://www.nuget.org/api/v2/package/Microsoft.Windows.CppWinRT/2.0.221121.5                                                                       %vendorPath%\Microsoft.Windows.CppWinRT
call :DOWNLOAD_AND_UNZIP https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.7.2212/dxc_2022_12_16.zip                                                  %vendorPath%\DirectXShaderCompiler
call :DOWNLOAD_AND_UNZIP https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.zip                                                                                    %vendorPath%\glfw
rem call :DOWNLOAD_AND_UNZIP https://github.com/wolfpld/tracy/releases/download/v0.11.1/windows-0.11.1.zip                                                                      %vendorPath%\tracyProfiler
call :DOWNLOAD_AND_UNZIP https://github.com/danmar/cppcheck/archive/2.16.0.zip                                                                                              %vendorPath%\cppcheck
call :DOWNLOAD_AND_UNZIP https://developer.nvidia.com/downloads/assets/tools/secure/nsight-aftermath-sdk/2025_1_0/windows/NVIDIA_Nsight_Aftermath_SDK_2025.1.0.25009.zip    %vendorPath%\NVIDIA_Nsight_Aftermath_SDK

robocopy "%vendorPath%\glfw\glfw-3.4" "%vendorPath%\glfw" /E /MOV

set vsVersion=
set cmakeGenerator=
set cmakeArch="x64"

echo Finding most recent cmake VS option
for /f "tokens=1,2,3,4 delims=," %%a in (../Engine/vsDevCmdVersions.txt) do (
    if "!vsDevCmd!" == "" (
        if exist "%%a" (
            SET cmakeGenerator="%%d"
            set vsVersion=%%b
            echo Selecting CMake generator '%%d'
        )
    )
)

echo Generate and Assimp
cd "%vendorPath%/assimp"
git clean -fxd build/CMakeCache.txt build/CMakeFiles
call cmake -S "./" -B "build" -G !cmakeGenerator! -A %cmakeArch%
cd "%currentDirectory%"
call "../Engine/Build_Solution.bat" "%vendorPath%/assimp/build/Assimp.sln" !vsVersion! Build Debug x64
call "../Engine/Build_Solution.bat" "%vendorPath%/assimp/build/Assimp.sln" !vsVersion! Build Release x64

echo Generate and spdlog
cd "%vendorPath%/spdlog"
git clean -fxd build/CMakeCache.txt build/CMakeFiles
call cmake -S "./" -B "build"  -G !cmakeGenerator! -A %cmakeArch% -D SPDLOG_BUILD_SHARED=ON
cd "%currentDirectory%"
call "../Engine/Build_Solution.bat" "%vendorPath%/spdlog/build/spdlog.sln" !vsVersion! Build Debug x64
call "../Engine/Build_Solution.bat" "%vendorPath%/spdlog/build/spdlog.sln" !vsVersion! Build Release x64


echo Generate and Build tracy
cd "%vendorPath%/tracy"
git clean -fxd build/CMakeCache.txt build/CMakeFiles
call cmake -S "./" -B "build" -G !cmakeGenerator! -A %cmakeArch% -D TRACY_STATIC=OFF -D TRACY_ON_DEMAND=ON
cd "%currentDirectory%"
call "../Engine/Build_Solution.bat" "%vendorPath%/tracy/build/Tracy.sln" !vsVersion! Build Debug x64
call "../Engine/Build_Solution.bat" "%vendorPath%/tracy/build/Tracy.sln" !vsVersion! Build Release x64

echo Generate and Build tracy profiler
cd "%vendorPath%/tracy/profiler"
git clean -fxd build/CMakeCache.txt build/CMakeFiles
call cmake -S "./" -B "build" -G !cmakeGenerator! -A %cmakeArch% -D TRACY_ON_DEMAND=ON
cd "%currentDirectory%"
call "../Engine/Build_Solution.bat" "%vendorPath%/tracy/profiler/build/tracy-profiler.sln" !vsVersion! Build Debug x64
call "../Engine/Build_Solution.bat" "%vendorPath%/tracy/profiler/build/tracy-profiler.sln" !vsVersion! Build Release x64


echo Generate JoltPhysics solution
cd "%vendorPath%/JoltPhysics/Build
if "!vsVersion!" == "vs2022" (
git clean -fxd VS2022_CL/CMakeCache.txt VS2022_CL/CMakeFiles
call cmake_vs2022_cl.bat -DUSE_STATIC_MSVC_RUNTIME_LIBRARY=OFF -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded$<$<CONFIG:Debug>:DebugDLL>$<$<CONFIG:Release>:DLL>$<$<CONFIG:Distribution>:DLL>" -Wno-dev
)
if "!vsVersion!" == "vs2026" (
    git clean -fxd VS2026_CL/CMakeCache.txt VS2026_CL/CMakeFiles
call cmake_vs2026_cl.bat -DUSE_STATIC_MSVC_RUNTIME_LIBRARY=OFF -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded$<$<CONFIG:Debug>:DebugDLL>$<$<CONFIG:Release>:DLL>$<$<CONFIG:Distribution>:DLL>" -Wno-dev
)
cd "%currentDirectory%"

echo Build JoltPhysics debug
if "!vsVersion!" == "vs2022" (
call "../Engine/Build_Solution.bat" "%vendorPath%/JoltPhysics/Build/VS2022_CL/JoltPhysics.sln" !vsVersion! Build Debug x64
)
if "!vsVersion!" == "vs2026" (
call "../Engine/Build_Solution.bat" "%vendorPath%/JoltPhysics/Build/VS2026_CL/JoltPhysics.sln" !vsVersion! Build Debug x64
)

echo Build JoltPhysics release
if "!vsVersion!" == "vs2022" (
call "../Engine/Build_Solution.bat" "%vendorPath%/JoltPhysics/Build/VS2022_CL/JoltPhysics.sln" !vsVersion! Build Release x64
)
if "!vsVersion!" == "vs2026" (
call "../Engine/Build_Solution.bat" "%vendorPath%/JoltPhysics/Build/VS2026_CL/JoltPhysics.sln" !vsVersion! Build Release x64
)

rem Generate FSR2 projects and build them.
cd "%vendorPath%\FidelityFX-FSR2\build"
SET FSR2GenerateSolutions="0"
if not exist "DX12" (
    SET FSR2GenerateSolutions="1"
)
if not exist "VK" (
    SET FSR2GenerateSolutions="1"
)

if %FSR2GenerateSolutions%=="1" (
    echo Generating FSR2 solutions
    call GenerateSolutions.bat

    echo Buildiing FSR2 DX12 solutions
    call "..\..\..\Build\Engine\Build_Solution.bat" "%vendorPath%\FidelityFX-FSR2\build\DX12\FSR2_Sample_DX12.sln" !vsVersion! Build Release x64
    call "..\..\..\Build\Engine\Build_Solution.bat" "%vendorPath%\FidelityFX-FSR2\build\DX12\FSR2_Sample_DX12.sln" !vsVersion! Build Debug x64

    echo Buildiing FSR2 Vulkan solutions
    call "..\..\..\Build\Engine\Build_Solution.bat" "%vendorPath%\FidelityFX-FSR2\build\VK\FSR2_Sample_VK.sln" !vsVersion! Build Release x64
    call "..\..\..\Build\Engine\Build_Solution.bat" "%vendorPath%\FidelityFX-FSR2\build\VK\FSR2_Sample_VK.sln" !vsVersion! Build Debug x64
)

rem Copy all downloaded and unziped lib/dll and built lib/dll into the deps folder. 
cd "..\..\..\Build\Dependencies"
call Copy_Vendor_Libs_To_Dependencies.bat

pause
exit 0

:DOWNLOAD_AND_UNZIP
set URL=%~1
set UNZIPLOC=%~2
set ZIP="%~dp0..\..\vendor\dependencies.zip"
echo Download URL: %URL%
echo Unzip location=%UNZIPLOC%

if exist "%ZIP%" (
    del /f /q "%ZIP%"
)

if not exist "%UNZIPLOC%" (
    powershell -Command "Invoke-WebRequest %URL% -OutFile %ZIP%"
    ::call ../Tools/UnZip.bat "%UNZIPLOC%" "%ZIP%"
    powershell Expand-Archive "%ZIP%" -DestinationPath "%UNZIPLOC%" -Force
    del /f /q "%ZIP%"
)