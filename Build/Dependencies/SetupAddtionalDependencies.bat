@echo off

:: Download pix
call :DOWNLOAD_AND_UNZIP https://www.nuget.org/api/v2/package/WinPixEventRuntime/1.0.220810001 F:\Users\Documents\SourceControl\Github\C++Porjects\Insight\vendor\winpixeventruntime
call :DOWNLOAD_AND_UNZIP https://www.nuget.org/api/v2/package/Microsoft.Direct3D.D3D12/1.706.4-preview F:\Users\Documents\SourceControl\Github\C++Porjects\Insight\vendor\Microsoft.Direct3D.D3D12

exit 0

:DOWNLOAD_AND_UNZIP
set URL=%~1
set UNZIPLOC=%~2
set ZIP=F:\Users\Documents\SourceControl\Github\C++Porjects\Insight\vendor\dependencies.zip
echo Download URL: %URL%
echo Unzip location=%UNZIPLOC%
powershell -Command "Invoke-WebRequest %URL% -OutFile %ZIP%"
call ../Tools/UnZip.bat "%UNZIPLOC%" "%ZIP%"
del /f /q "%ZIP%"