@echo off
@setlocal enableextensions
@setlocal enabledelayedexpansion
@cd /d "%~dp0"

SET vsVersion=%1
SET msBuildType=%2

echo Starting dependencies builds

echo Generating premake projects and solution
call ..\..\vendor\premake\premake5.exe --file=premake5.lua %vsVersion% 
echo Solution generated

if "%msBuildType%" == "" (
    echo Solution has not been built as no build type was given
    GOTO END
)

set vsDevCmd=
echo Finding most recent VS option
for /f "tokens=1,2 delims=," %%a in (../Engine/vsDevCmdVersions.txt) do (
    if "!vsDevCmd!" == "" (
        if exist "%%a" (
            SET vsDevCmd=%%a
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

:MSBUILD
SET solutionFile="../../Engine/Vendor/Dependencies.sln"

msbuild -maxCpuCount /t:!msBuildType! /p:Configuration="Debug" /p:Platform=Win64 %solutionFile%
msbuild -maxCpuCount /t:!msBuildType! /p:Configuration="Release" /p:Platform=Win64 %solutionFile%
GOTO END

:END
pause