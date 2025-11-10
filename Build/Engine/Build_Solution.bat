@echo off
@setlocal enableextensions
@setlocal enabledelayedexpansion
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

set MSBuildTypes="Build Rebuild"
call :ValididateInput %MSBuildTypes% "%msBuildType%" "Invalid build type, valid build types are:" 
set MSConfigurationTypes="Debug Release"
call :ValididateInput %MSConfigurationTypes% "%configuration%" "Invalid configuration type, valid configuration types are:"

set MSPlatformTypes="win64, x64"
call :ValididateInput %MSPlatformTypes% "%platform%" "Invalid platform type, valid platform types are:"

set vsSolutionExtension=
set vsDevCmd=
echo Finding most recent VS option
for /f "tokens=1,2,3 delims=," %%a in (vsDevCmdVersions.txt) do (
    if "!vsDevCmd!" == "" (
        if exist "%%a" (
            SET vsDevCmd=%%a
            if "%%b" == "vs2026" (
                set vsSolutionExtension=x
            )
        )
    )
)

if "!vsDevCmd!" == "" (
    echo No valid VSDevCmd found.
    GOTO END
) else (
    echo Found VSDevCmd !vsDevCmd!
    call "!vsDevCmd!"
    GOTO MSBUILD
)

:HELP
echo Parameter 1 = "Solution file path"
echo Parameter 2 = "VS version (vs2019, vs2022)"
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

set solution=!solution!!vsSolutionExtension!
msbuild -maxCpuCount /t:%msBuildType% /p:Configuration=%configuration% /p:Platform=%platform% %msbuildOutDirectory% !solution!
GOTO END

:ValididateInput
set array=%~1
set inputValue=%~2
set errorMessage=%~3
set foundValidInput=0

for %%i in (%array%) do ( 
    if %%i EQU %inputValue% ( 
        set foundValidInput=1
    ) 
)

if %foundValidInput% == 0 (
    echo %errorMessage%
    for %%i in (%array%) do (
        echo %%i
    )
    echo Input: %inputValue%
    set /p DUMMY=Hit ENTER to close %foundValidInput% ...
    exit 0
)

:END
exit /b