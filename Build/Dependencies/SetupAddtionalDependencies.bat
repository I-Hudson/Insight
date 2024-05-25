@echo off

set vendorPath=%cd%\..\..\vendor

:: Download pix
call :DOWNLOAD_AND_UNZIP https://www.nuget.org/api/v2/package/WinPixEventRuntime/1.0.220810001                                      %vendorPath%\winpixeventruntime
call :DOWNLOAD_AND_UNZIP https://www.nuget.org/api/v2/package/Microsoft.Direct3D.D3D12/1.706.4-preview                              %vendorPath%\Microsoft.Direct3D.D3D12
call :DOWNLOAD_AND_UNZIP https://www.nuget.org/api/v2/package/Microsoft.VCRTForwarders.140/1.0.7                                    %vendorPath%\Microsoft.VCRTForwarders.140
call :DOWNLOAD_AND_UNZIP https://www.nuget.org/api/v2/package/Microsoft.Windows.CppWinRT/2.0.221121.5                               %vendorPath%\Microsoft.Windows.CppWinRT
call :DOWNLOAD_AND_UNZIP https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.7.2212/dxc_2022_12_16.zip          %vendorPath%\DirectXShaderCompiler
call :DOWNLOAD_AND_UNZIP https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.zip                                            %vendorPath%\glfw
robocopy "%vendorPath%\glfw\glfw-3.4" "%vendorPath%\glfw" /E /MOV

echo Generate JoltPhysics solution
call ../../vendor/JoltPhysics/Build/cmake_vs2022_cl.bat
echo Build JoltPhysics debug

call ../Engine/Build_Solution.bat %vendorPath%/JoltPhysics/Build/VS2022_CL/JoltPhysics.sln vs2022 Build Debug x64
echo Build JoltPhysics release
call ../Engine/Build_Solution.bat %vendorPath%/JoltPhysics/Build/VS2022_CL/JoltPhysics.sln vs2022 Build Release x64

rem Genreate FSR2 projects and build them.
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
    call ..\..\..\Build\Engine\Build_Solution.bat %vendorPath%\FidelityFX-FSR2\build\DX12\FSR2_Sample_DX12.sln vs2022 Build Release x64
    call ..\..\..\Build\Engine\Build_Solution.bat %vendorPath%\FidelityFX-FSR2\build\DX12\FSR2_Sample_DX12.sln vs2022 Build Debug x64

    echo Buildiing FSR2 Vulkan solutions
    call ..\..\..\Build\Engine\Build_Solution.bat %vendorPath%\FidelityFX-FSR2\build\VK\FSR2_Sample_VK.sln vs2022 Build Release x64
    call ..\..\..\Build\Engine\Build_Solution.bat %vendorPath%\FidelityFX-FSR2\build\VK\FSR2_Sample_VK.sln vs2022 Build Debug x64
)

rem Copy all downloaded and unziped lib/dll and built lib/dll into the deps folder. 
cd "..\..\..\Build\Dependencies"
call Copy_Vendor_Libs_To_Dependencies.bat

pause
exit 0

:DOWNLOAD_AND_UNZIP
set URL=%~1
set UNZIPLOC=%~2
set ZIP=%cd%\..\..\vendor\dependencies.zip
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