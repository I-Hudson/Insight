@echo off@echo off
@setlocal enabledelayedexpansion

rem Capture the start time
set "STARTTIME=%TIME%"

set cleanRepos=%1
echo Clean repos: %cleanRepos%

set vendorPath=%~dp0..\..\vendor
set currentDirectory=%~dp0

:: Download pix
call :DOWNLOAD_AND_UNZIP https://www.nuget.org/api/v2/package/WinPixEventRuntime/1.0.220810001                                                                              %vendorPath%\winpixeventruntime
call :DOWNLOAD_AND_UNZIP https://www.nuget.org/api/v2/package/Microsoft.Direct3D.D3D12/1.717.1-preview                                                                      %vendorPath%\Microsoft.Direct3D.D3D12
call :DOWNLOAD_AND_UNZIP https://www.nuget.org/api/v2/package/Microsoft.VCRTForwarders.140/1.0.7                                                                            %vendorPath%\Microsoft.VCRTForwarders.140
call :DOWNLOAD_AND_UNZIP https://www.nuget.org/api/v2/package/Microsoft.Windows.CppWinRT/2.0.221121.5                                                                       %vendorPath%\Microsoft.Windows.CppWinRT
call :DOWNLOAD_AND_UNZIP https://www.nuget.org/api/v2/package/Microsoft.GameInput/3.5.283                                                                                   %vendorPath%\Microsoft.GameInput
call :DOWNLOAD_AND_UNZIP https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.7.2212/dxc_2022_12_16.zip                                                  %vendorPath%\DirectXShaderCompiler
call :DOWNLOAD_AND_UNZIP https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.zip                                                                                    %vendorPath%\glfw
rem call :DOWNLOAD_AND_UNZIP https://github.com/wolfpld/tracy/releases/download/v0.11.1/windows-0.11.1.zip                                                                      %vendorPath%\tracyProfiler
call :DOWNLOAD_AND_UNZIP https://github.com/danmar/cppcheck/archive/2.16.0.zip                                                                                              %vendorPath%\cppcheck
call :DOWNLOAD_AND_UNZIP https://developer.nvidia.com/downloads/assets/tools/secure/nsight-aftermath-sdk/2025_1_0/windows/NVIDIA_Nsight_Aftermath_SDK_2025.1.0.25009.zip    %vendorPath%\NVIDIA_Nsight_Aftermath_SDK

robocopy "%vendorPath%\glfw\glfw-3.4" "%vendorPath%\glfw" /E /MOV

set cmakeArch="x64"

call ../Engine/bat/SetVSCompilerVars.bat

if "%VS_COMPILER_VERSION%" == "" (
    echo No MSVC compiler was found
    pause
    exit -1
)

echo Cmake Generator '%CMAKE_GENERATOR%'

title Generate and Build Assimp
echo Generate and Assimp
cd "%vendorPath%/assimp"
if "%cleanRepos%" == "true" (
    git clean -fxd build/CMakeCache.txt build/CMakeFiles
)
call cmake -S "./" -B "build" -G %CMAKE_GENERATOR% -A %cmakeArch%
cd "%currentDirectory%"
call "../Engine/Build_Solution.bat" "%vendorPath%/assimp/build/Assimp.sln" %VS_COMPILER_VERSION% Build Debug x64
call "../Engine/Build_Solution.bat" "%vendorPath%/assimp/build/Assimp.sln" %VS_COMPILER_VERSION% Build Release x64

title Generate and Build spdlog
echo Generate and spdlog
cd "%vendorPath%/spdlog"
if "%cleanRepos%" == "true" (
    git clean -fxd build/CMakeCache.txt build/CMakeFiles
)
call cmake -S "./" -B "build"  -G %CMAKE_GENERATOR% -A %cmakeArch% -D SPDLOG_BUILD_SHARED=ON
cd "%currentDirectory%"
call "../Engine/Build_Solution.bat" "%vendorPath%/spdlog/build/spdlog.sln" %VS_COMPILER_VERSION% Build Debug x64
call "../Engine/Build_Solution.bat" "%vendorPath%/spdlog/build/spdlog.sln" %VS_COMPILER_VERSION% Build Release x64

title Generate and Build tracy
echo Generate and Build tracy
cd "%vendorPath%/tracy"
if "%cleanRepos%" == "true" (
    git clean -fxd build/CMakeCache.txt build/CMakeFiles
)
call cmake -S "./" -B "build" -G %CMAKE_GENERATOR% -A %cmakeArch% -D TRACY_STATIC=OFF -D TRACY_ON_DEMAND=ON -D TRACY_ENABLE=ON
cd "%currentDirectory%"
call "../Engine/Build_Solution.bat" "%vendorPath%/tracy/build/Tracy.sln" %VS_COMPILER_VERSION% Build Debug x64
call "../Engine/Build_Solution.bat" "%vendorPath%/tracy/build/Tracy.sln" %VS_COMPILER_VERSION% Build Release x64

title Generate and Build tracy profiler
echo Generate and Build tracy profiler
cd "%vendorPath%/tracy/profiler"
if "%cleanRepos%" == "true" (
    git clean -fxd ./
    rem build/CMakeCache.txt build/CMakeFiles
)
call cmake -S "./" -B "build" -G %CMAKE_GENERATOR% -A %cmakeArch% -D TRACY_ON_DEMAND=ON
cd "%currentDirectory%"
call "../Engine/Build_Solution.bat" "%vendorPath%/tracy/profiler/build/tracy-profiler.sln" %VS_COMPILER_VERSION% Build Debug x64
call "../Engine/Build_Solution.bat" "%vendorPath%/tracy/profiler/build/tracy-profiler.sln" %VS_COMPILER_VERSION% Build Release x64

title Generate and Build JoltPhysics
echo Generate JoltPhysics solution
cd "%vendorPath%/JoltPhysics/Build
if %VS_COMPILER_VERSION% == "vs2022" (
    if "%cleanRepos%" == "true" (
        git clean -fxd VS2022_CL/CMakeCache.txt VS2022_CL/CMakeFiles
    )
call cmake_vs2022_cl.bat -DUSE_STATIC_MSVC_RUNTIME_LIBRARY=OFF -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded$<$<CONFIG:Debug>:DebugDLL>$<$<CONFIG:Release>:DLL>$<$<CONFIG:Distribution>:DLL>" -Wno-dev
)
if %VS_COMPILER_VERSION% == "vs2026" (
    if "%cleanRepos%" == "true" (
        git clean -fxd VS2026_CL/CMakeCache.txt VS2026_CL/CMakeFiles
    )
call cmake_vs2026_cl.bat -DUSE_STATIC_MSVC_RUNTIME_LIBRARY=OFF -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded$<$<CONFIG:Debug>:DebugDLL>$<$<CONFIG:Release>:DLL>$<$<CONFIG:Distribution>:DLL>" -Wno-dev
)
cd "%currentDirectory%"

echo Build JoltPhysics
if %VS_COMPILER_VERSION% == "vs2022" (
    call "../Engine/Build_Solution.bat" "%vendorPath%/JoltPhysics/Build/VS2022_CL/JoltPhysics.sln" %VS_COMPILER_VERSION% Build Debug x64
    call "../Engine/Build_Solution.bat" "%vendorPath%/JoltPhysics/Build/VS2022_CL/JoltPhysics.sln" %VS_COMPILER_VERSION% Build Release x64
)
if %VS_COMPILER_VERSION% == "vs2026" (
    call "../Engine/Build_Solution.bat" "%vendorPath%/JoltPhysics/Build/VS2026_CL/JoltPhysics.sln" %VS_COMPILER_VERSION% Build Debug x64
    call "../Engine/Build_Solution.bat" "%vendorPath%/JoltPhysics/Build/VS2026_CL/JoltPhysics.sln" %VS_COMPILER_VERSION% Build Release x64
)

title Generate and Build FSR
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
    call "..\..\..\Build\Engine\Build_Solution.bat" "%vendorPath%\FidelityFX-FSR2\build\DX12\FSR2_Sample_DX12.sln" %VS_COMPILER_VERSION% Build Release x64
    call "..\..\..\Build\Engine\Build_Solution.bat" "%vendorPath%\FidelityFX-FSR2\build\DX12\FSR2_Sample_DX12.sln" %VS_COMPILER_VERSION% Build Debug x64

    echo Buildiing FSR2 Vulkan solutions
    call "..\..\..\Build\Engine\Build_Solution.bat" "%vendorPath%\FidelityFX-FSR2\build\VK\FSR2_Sample_VK.sln" %VS_COMPILER_VERSION% Build Release x64
    call "..\..\..\Build\Engine\Build_Solution.bat" "%vendorPath%\FidelityFX-FSR2\build\VK\FSR2_Sample_VK.sln" %VS_COMPILER_VERSION% Build Debug x64
)

title Generate and Build Compressonator
echo Build Compressonator
cd "%vendorPath%/Compressonator/build"
if "%cleanRepos%" == "true" (
    git clean -fxd build/CMakeCache.txt build/CMakeFiles
)
call build/windows_build_sdk_cmake.bat
cd "%currentDirectory%"

title Copy all dependencies to deps folder
rem Copy all downloaded and unziped lib/dll and built lib/dll into the deps folder. 
cd "..\..\..\Build\Dependencies"
call Copy_Vendor_Libs_To_Dependencies.bat

rem Capture the end time
set "ENDTIME=%TIME%"

call :ECHO_EXECUTION_TIME %STARTIME% %ENDTIME%

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

:ECHO_EXECUTION_TIME
rem Format times to strip potential leading spaces in hours (e.g., " 9:30:00")
set "STARTTIME=%~1: =0%"
set "ENDTIME=%~2: =0%"

rem Parse Start Time into parts
for /f "tokens=1-4 delims=:.," %%a in ("%STARTTIME%") do (
    set /a "start_h=100%%a %% 100", "start_m=100%%b %% 100", "start_s=100%%c %% 100", "start_cs=100%%d %% 100"
)

rem Parse End Time into parts
for /f "tokens=1-4 delims=:.," %%a in ("%ENDTIME%") do (
    set /a "end_h=100%%a %% 100", "end_m=100%%b %% 100", "end_s=100%%c %% 100", "end_cs=100%%d %% 100"
)

:: Convert both times into total centiseconds
set /a "start_total=(start_h * 360000) + (start_m * 6000) + (start_s * 100) + start_cs"
set /a "end_total=(end_h * 360000) + (end_m * 6000) + (end_s * 100) + end_cs"

:: Calculate the difference
set /a "elapsed_total=end_total - start_total"

:: Handle midnight crossover (if end time is less than start time)
if %elapsed_total% lss 0 set /a "elapsed_total+=8640000"

:: Format the final output back into seconds and centiseconds
set /a "elapsed_s=elapsed_total / 100"
set /a "elapsed_cs=elapsed_total %% 100"

:: Ensure centiseconds have a leading zero if needed
if %elapsed_cs% lss 10 set "elapsed_cs=0%elapsed_cs%"

echo ==========================================
echo Execution started:  %STARTTIME%
echo Execution finished: %ENDTIME%
echo Total Elapsed Time: %elapsed_s%.%elapsed_cs% seconds
echo ==========================================
