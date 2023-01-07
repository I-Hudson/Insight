@echo off

set vendorPath=%cd%\..\..\vendor

:: Download pix
call :DOWNLOAD_AND_UNZIP https://www.nuget.org/api/v2/package/WinPixEventRuntime/1.0.220810001                                      %vendorPath%\winpixeventruntime
call :DOWNLOAD_AND_UNZIP https://www.nuget.org/api/v2/package/Microsoft.Direct3D.D3D12/1.706.4-preview                              %vendorPath%\Microsoft.Direct3D.D3D12
call :DOWNLOAD_AND_UNZIP https://www.nuget.org/api/v2/package/Microsoft.VCRTForwarders.140/1.0.7                                    %vendorPath%\Microsoft.VCRTForwarders.140
call :DOWNLOAD_AND_UNZIP https://www.nuget.org/api/v2/package/Microsoft.Windows.CppWinRT/2.0.221121.5                               %vendorPath%\Microsoft.Windows.CppWinRT
call :DOWNLOAD_AND_UNZIP https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.7.2212/dxc_2022_12_16.zip          %vendorPath%\DirectXShaderCompiler

pause
exit 0

:DOWNLOAD_AND_UNZIP
set URL=%~1
set UNZIPLOC=%~2
set ZIP=F:\Users\Documents\SourceControl\Github\C++Porjects\Insight\vendor\dependencies.zip
echo Download URL: %URL%
echo Unzip location=%UNZIPLOC%
if not exist "%UNZIPLOC%" (
    powershell -Command "Invoke-WebRequest %URL% -OutFile %ZIP%"
    ::call ../Tools/UnZip.bat "%UNZIPLOC%" "%ZIP%"
    powershell Expand-Archive "%ZIP%" -DestinationPath "%UNZIPLOC%"
    del /f /q "%ZIP%"
)