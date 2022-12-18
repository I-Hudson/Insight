@echo off 

call :COPY_TO_DEBUG_DEPS "..\..\vendor\FidelityFX-FSR2\bin\ffx_fsr2_api\ffx_fsr2_api_x64d.lib"						        "lib"
call :COPY_TO_DEBUG_DEPS "..\..\vendor\FidelityFX-FSR2\bin\ffx_fsr2_api\ffx_fsr2_api_vk_x64d.lib"					        "lib"
call :COPY_TO_DEBUG_DEPS "..\..\vendor\FidelityFX-FSR2\bin\ffx_fsr2_api\ffx_fsr2_api_dx12_x64d.lib"					        "lib"

call :COPY_TO_RELEASE_DEPS "..\..\vendor\FidelityFX-FSR2\bin\ffx_fsr2_api\ffx_fsr2_api_x64.lib"						        "lib"
call :COPY_TO_RELEASE_DEPS "..\..\vendor\FidelityFX-FSR2\bin\ffx_fsr2_api\ffx_fsr2_api_vk_x64.lib"					        "lib"
call :COPY_TO_RELEASE_DEPS "..\..\vendor\FidelityFX-FSR2\bin\ffx_fsr2_api\ffx_fsr2_api_dx12_x64.lib"					    "lib"

call :COPY_TO_DEBUG_DEPS "..\..\vendor\NVIDIA_Nsight_Aftermath_SDK_2022.1.0.22080\lib\x64\GFSDK_Aftermath_Lib.x64.lib"      "lib"
call :COPY_TO_DEBUG_DEPS "..\..\vendor\NVIDIA_Nsight_Aftermath_SDK_2022.1.0.22080\lib\x64\GFSDK_Aftermath_Lib.x64.dll"		"dll"

call :COPY_TO_RELEASE_DEPS "..\..\vendor\NVIDIA_Nsight_Aftermath_SDK_2022.1.0.22080\lib\x64\GFSDK_Aftermath_Lib.x64.lib"	"lib"
call :COPY_TO_RELEASE_DEPS "..\..\vendor\NVIDIA_Nsight_Aftermath_SDK_2022.1.0.22080\lib\x64\GFSDK_Aftermath_Lib.x64.dll"	"dll"

call :COPY_TO_DEBUG_DEPS "..\..\vendor\winpixeventruntime\bin\x64\WinPixEventRuntime.lib"                                   "lib"
call :COPY_TO_DEBUG_DEPS "..\..\vendor\winpixeventruntime\bin\x64\WinPixEventRuntime.dll"		                            "dll"
call :COPY_TO_RELEASE_DEPS "..\..\vendor\winpixeventruntime\bin\x64\WinPixEventRuntime.lib"                                 "lib"
call :COPY_TO_RELEASE_DEPS "..\..\vendor\winpixeventruntime\bin\x64\WinPixEventRuntime.dll"		                            "dll"

pause

:COPY_TO_DEBUG_DEPS
set lib_file="%cd%\%~1"
set output="%cd%\..\..\deps\Debug-windows-x86_64\%~2\"
xcopy /Y /B %lib_file% %output%

:COPY_TO_RELEASE_DEPS
set output="%cd%\..\..\deps\Release-windows-x86_64\%~2\"
set lib_file="%cd%\%~1"
xcopy /Y /B %lib_file% %output%
