@echo off 
call :COPY_TO_DEBUG_DEPS "..\..\vendor\tracy\build\Debug\TracyClient.dll"						                            "dll"
call :COPY_TO_DEBUG_DEPS "..\..\vendor\tracy\build\Debug\TracyClient.lib"						                            "lib"
call :COPY_TO_DEBUG_DEPS "..\..\vendor\tracy\build\Debug\TracyClient.pdb"						                            "pdb"

call :COPY_TO_DEBUG_DEPS "..\..\vendor\tracy\build\Release\TracyClient.dll"						                            "dll"
call :COPY_TO_DEBUG_DEPS "..\..\vendor\tracy\build\Release\TracyClient.lib"						                            "lib"
call :COPY_TO_DEBUG_DEPS "..\..\vendor\tracy\build\Release\TracyClient.pdb"						                            "pdb"

call :COPY_TO_DEBUG_DEPS "..\..\vendor\JoltPhysics\Build\VS2022_CL\Debug\Jolt.lib"						                    "lib"
call :COPY_TO_DEBUG_DEPS "..\..\vendor\JoltPhysics\Build\VS2022_CL\Debug\Jolt.pdb"						                    "pdb"

call :COPY_TO_DEBUG_DEPS "..\..\vendor\JoltPhysics\Build\VS2022_CL\Release\Jolt.lib"						                "lib"
call :COPY_TO_DEBUG_DEPS "..\..\vendor\JoltPhysics\Build\VS2022_CL\Release\Jolt.pdb"						                "pdb"

call :COPY_TO_DEBUG_DEPS "..\..\vendor\FidelityFX-FSR2\bin\ffx_fsr2_api\ffx_fsr2_api_x64d.lib"						        "lib"
call :COPY_TO_DEBUG_DEPS "..\..\vendor\FidelityFX-FSR2\bin\ffx_fsr2_api\ffx_fsr2_api_vk_x64d.lib"					        "lib"
call :COPY_TO_DEBUG_DEPS "..\..\vendor\FidelityFX-FSR2\bin\ffx_fsr2_api\ffx_fsr2_api_dx12_x64d.lib"					        "lib"

call :COPY_TO_RELEASE_DEPS "..\..\vendor\FidelityFX-FSR2\bin\ffx_fsr2_api\ffx_fsr2_api_x64.lib"						        "lib"
call :COPY_TO_RELEASE_DEPS "..\..\vendor\FidelityFX-FSR2\bin\ffx_fsr2_api\ffx_fsr2_api_vk_x64.lib"					        "lib"
call :COPY_TO_RELEASE_DEPS "..\..\vendor\FidelityFX-FSR2\bin\ffx_fsr2_api\ffx_fsr2_api_dx12_x64.lib"					    "lib"

call :COPY_TO_DEBUG_DEPS "..\..\vendor\NVIDIA_Nsight_Aftermath_SDK\lib\x64\GFSDK_Aftermath_Lib.x64.lib"                     "lib"
call :COPY_TO_DEBUG_DEPS "..\..\vendor\NVIDIA_Nsight_Aftermath_SDK\lib\x64\GFSDK_Aftermath_Lib.x64.dll"		                "dll"

call :COPY_TO_RELEASE_DEPS "..\..\vendor\NVIDIA_Nsight_Aftermath_SDK\lib\x64\GFSDK_Aftermath_Lib.x64.lib"	                "lib"
call :COPY_TO_RELEASE_DEPS "..\..\vendor\NVIDIA_Nsight_Aftermath_SDK\lib\x64\GFSDK_Aftermath_Lib.x64.dll"	                "dll"

call :COPY_TO_DEBUG_DEPS "..\..\vendor\winpixeventruntime\bin\x64\WinPixEventRuntime.lib"                                   "lib"
call :COPY_TO_DEBUG_DEPS "..\..\vendor\winpixeventruntime\bin\x64\WinPixEventRuntime.dll"		                            "dll"
call :COPY_TO_RELEASE_DEPS "..\..\vendor\winpixeventruntime\bin\x64\WinPixEventRuntime.lib"                                 "lib"
call :COPY_TO_RELEASE_DEPS "..\..\vendor\winpixeventruntime\bin\x64\WinPixEventRuntime.dll"		                            "dll"

call :COPY_TO_DEBUG_DEPS "..\..\vendor\Microsoft.Direct3D.D3D12\build\native\bin\x64\D3D12Core.dll"                         "dll\D3D12"
call :COPY_TO_DEBUG_DEPS "..\..\vendor\Microsoft.Direct3D.D3D12\build\native\bin\x64\d3d12SDKLayers.dll"                    "dll\D3D12"
call :COPY_TO_RELEASE_DEPS "..\..\vendor\Microsoft.Direct3D.D3D12\build\native\bin\x64\D3D12Core.dll"                       "dll\D3D12"

call :COPY_TO_DEBUG_DEPS "..\..\vendor\Microsoft.VCRTForwarders.140\runtimes\win10-x64\native\debug\concrt140d_app.dll"      "dll"
call :COPY_TO_DEBUG_DEPS "..\..\vendor\Microsoft.VCRTForwarders.140\runtimes\win10-x64\native\debug\msvcp140_1d_app.dll"     "dll"
call :COPY_TO_DEBUG_DEPS "..\..\vendor\Microsoft.VCRTForwarders.140\runtimes\win10-x64\native\debug\msvcp140_2d_app.dll"     "dll"
call :COPY_TO_DEBUG_DEPS "..\..\vendor\Microsoft.VCRTForwarders.140\runtimes\win10-x64\native\debug\msvcp140d_app.dll"       "dll"
call :COPY_TO_DEBUG_DEPS "..\..\vendor\Microsoft.VCRTForwarders.140\runtimes\win10-x64\native\debug\vcamp140d_app.dll"       "dll"
call :COPY_TO_DEBUG_DEPS "..\..\vendor\Microsoft.VCRTForwarders.140\runtimes\win10-x64\native\debug\vccorlib140d_app.dll"    "dll"
call :COPY_TO_DEBUG_DEPS "..\..\vendor\Microsoft.VCRTForwarders.140\runtimes\win10-x64\native\debug\vcomp140d_app.dll"       "dll"
call :COPY_TO_DEBUG_DEPS "..\..\vendor\Microsoft.VCRTForwarders.140\runtimes\win10-x64\native\debug\vcruntime140_1d_app.dll" "dll"
call :COPY_TO_DEBUG_DEPS "..\..\vendor\Microsoft.VCRTForwarders.140\runtimes\win10-x64\native\debug\vcruntime140d_app.dll"   "dll"

call :COPY_TO_RELEASE_DEPS "..\..\vendor\Microsoft.VCRTForwarders.140\runtimes\win10-x64\native\release\concrt140_app.dll"      "dll"
call :COPY_TO_RELEASE_DEPS "..\..\vendor\Microsoft.VCRTForwarders.140\runtimes\win10-x64\native\release\msvcp140_1_app.dll"     "dll"
call :COPY_TO_RELEASE_DEPS "..\..\vendor\Microsoft.VCRTForwarders.140\runtimes\win10-x64\native\release\msvcp140_2_app.dll"     "dll"
call :COPY_TO_RELEASE_DEPS "..\..\vendor\Microsoft.VCRTForwarders.140\runtimes\win10-x64\native\release\msvcp140_app.dll"       "dll"
call :COPY_TO_RELEASE_DEPS "..\..\vendor\Microsoft.VCRTForwarders.140\runtimes\win10-x64\native\release\vcamp140_app.dll"       "dll"
call :COPY_TO_RELEASE_DEPS "..\..\vendor\Microsoft.VCRTForwarders.140\runtimes\win10-x64\native\release\vccorlib140_app.dll"    "dll"
call :COPY_TO_RELEASE_DEPS "..\..\vendor\Microsoft.VCRTForwarders.140\runtimes\win10-x64\native\release\vcomp140_app.dll"       "dll"
call :COPY_TO_RELEASE_DEPS "..\..\vendor\Microsoft.VCRTForwarders.140\runtimes\win10-x64\native\release\vcruntime140_1_app.dll" "dll"
call :COPY_TO_RELEASE_DEPS "..\..\vendor\Microsoft.VCRTForwarders.140\runtimes\win10-x64\native\release\vcruntime140_app.dll"   "dll"

call :COPY_TO_DEBUG_DEPS "..\..\vendor\Microsoft.Windows.CppWinRT\build\native\lib\x64\cppwinrt_fast_forwarder.lib"             "lib"
call :COPY_TO_RELEASE_DEPS "..\..\vendor\Microsoft.Windows.CppWinRT\build\native\lib\x64\cppwinrt_fast_forwarder.lib"           "lib"

call :COPY_TO_DEBUG_DEPS "..\..\vendor\DirectXShaderCompiler\lib\x64\dxcompiler.lib"             "lib"
call :COPY_TO_DEBUG_DEPS "..\..\vendor\DirectXShaderCompiler\bin\x64\dxcompiler.dll"             "dll"
call :COPY_TO_DEBUG_DEPS "..\..\vendor\DirectXShaderCompiler\bin\x64\dxil.dll"                   "dll"

call :COPY_TO_RELEASE_DEPS "..\..\vendor\DirectXShaderCompiler\lib\x64\dxcompiler.lib"           "lib"
call :COPY_TO_RELEASE_DEPS "..\..\vendor\DirectXShaderCompiler\bin\x64\dxcompiler.dll"           "dll"
call :COPY_TO_RELEASE_DEPS "..\..\vendor\DirectXShaderCompiler\bin\x64\dxil.dll"                 "dll"

call :COPY_TO_DEBUG_DEPS "..\..\vendor\SimplygonSDK_10.1.11000.0\Simplygon.dll"                  "dll"
call :COPY_TO_DEBUG_DEPS "..\..\vendor\SimplygonSDK_10.1.11000.0\SimplygonCWrapper.dll"          "dll"

call :COPY_TO_RELEASE_DEPS "..\..\vendor\SimplygonSDK_10.1.11000.0\Simplygon.dll"                "dll"
call :COPY_TO_RELEASE_DEPS "..\..\vendor\SimplygonSDK_10.1.11000.0\SimplygonCWrapper.dll"        "dll"

call :COPY_TO_RELEASE_DEPS "..\..\vendor\SimplygonSDK_10.1.11000.0\Simplygon.dll"                "dll"
pause

:COPY_TO_DEBUG_DEPS
set lib_file="%~dp0%~1"
set output="%~dp0..\..\deps\Debug-windows-x86_64\%~2\"
if not exist %output% (
    mkdir %output%
)
xcopy /Y /B %lib_file% %output%

:COPY_TO_RELEASE_DEPS
set output="%~dp0..\..\deps\Release-windows-x86_64\%~2\"
set lib_file="%~dp0%~1"
if not exist %output% (
    mkdir %output%
)
xcopy /Y /B %lib_file% %output%
