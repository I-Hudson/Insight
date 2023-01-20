@echo off
@setlocal enableextensions
@cd /d "%~dp0"

SET solution=%1
SET vsVersion=%2
SET msBuildType=%3
SET configuration=%4
SET platform=%5
SET outDirectory=%6

if "%solution%" == "help" GOTO HELP

if "%vsVersion%" == "" GOTO END
if "%msBuildType%" == "" GOTO END
if "%configuration%" == "" GOTO END
if "%platform%" == "" GOTO END

SET vsDevCmd2022="C:\Program Files\Microsoft Visual Studio\2022\Preview\Common7\Tools\VsDevCmd.bat"
SET vsDevCmd2019="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

if exist %vsDevCmd2022% ( 
    echo Found vs 2022 dev cmd.
    call %vsDevCmd2022%
    GOTO MSBUILD
    )
if exist %vsDevCmd2019% (
    echo Found vs 2019 dev cmd.
    call %vsDevCmd2019%
    GOTO MSBUILD
) 
else (
    echo No valid VSDevCmd found.
    GOTO END
)
GOTO END

:HELP
echo Parameter 1 = "Solution file path"
echo Parameter 2 = "VS version (vs 2019, vs 2022)"
echo Parameter 3 = "build type (Build, Rebuid)"
echo Parameter 4 = "Configuration (Debug, Release)"
echo Parameter 5 = "Platform (win64, unix)"
echo Parameter 6 = "(Optional) Ouptup Path"
GOTO END

:MSBUILD
echo Start project build.
set msbuildOutDirectory=""

echo Build type '%msBuildType%'
echo Configuration '%configuration%'
echo Platform '%platform%'
if not "%outDirectory%" == "" ( 
    set msbuildOutDirectory=/p:OutDir=%outDirectory% 
    echo Out directory '%outDirectory%'
)

msbuild -maxCpuCount /t:%msBuildType% /p:Configuration=%configuration% /p:Platform=%platform% %msbuildOutDirectory% %solution% 
GOTO END

:END