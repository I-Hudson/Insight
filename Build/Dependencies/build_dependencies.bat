@echo off
@setlocal enableextensions
@cd /d "%~dp0"

SET vsVersion=%1
SET msBuildType=%2

echo Starting dependencies builds

echo Generating premake projects and solution
call ..\..\vendor\premake\premake5.exe %vsVersion% 
echo Solution generated

if "%msBuildType%" == "" (
    echo Solution has not been built as no build type was given
    GOTO END
)

SET vsDevCmd2022Preview="C:\Program Files\Microsoft Visual Studio\2022\Preview\Common7\Tools\VsDevCmd.bat"
SET vsDevCmd2022="C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
SET vsDevCmd2019="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

if exist %vsDevCmd2022Preview% (
    echo VSDevCmd 2022 Preview found.
    call %vsDevCmd2022Preview%
    GOTO MSBUILD
    )
if exist %vsDevCmd2022% ( 
    echo VSDevCmd 2022 found.
    call %vsDevCmd2022%
    GOTO MSBUILD
    )
if exist %vsDevCmd2019% (
    echo VSDevCmd 2019 found.
    call %vsDevCmd2019%
    GOTO MSBUILD
) 

echo No valid VSDevCmd found.
GOTO END

:MSBUILD
SET solutionFile="../../Engine/Vendor/Dependencies.sln"
msbuild -maxCpuCount /t:%msBuildType% /p:Configuration="Debug" /p:Platform=Win64 %solutionFile%
msbuild -maxCpuCount /t:%msBuildType% /p:Configuration="Release" /p:Platform=Win64 %solutionFile%
GOTO END

:END
pause