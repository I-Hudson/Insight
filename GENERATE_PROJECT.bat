@echo off
@setlocal enableextensions
@cd /d "%~dp0"


call vendor\premake\premake5.exe vs2019
echo Project sucessfully generated

set output_debug=%cd%\bin\Debug-windows-x86_64\InsightEditor
set output_release=%cd%\bin\Release-windows-x86_64\InsightEditor
set output_testing=%cd%\bin\Testing-windows-x86_64\InsightEditor

if exist "%output_debug%" (
    echo "Creating symlinks debug"
    CALL :SymlinkData "%output_debug%\Resources", "%cd%\Resources"
) else (
    echo "Dir %output_debug% has not been created."
)

if exist "%output_release%" (
echo "Creating symlinks release"
CALL :SymlinkData "%output_release%\Resources", "%cd%\Resources"
) else (
    echo "Dir %output_release% has not been created."
)

if exist "%output_testing%" (
echo "Creating symlinks release"
CALL :SymlinkData "%output_testing%\Resources", "%cd%\Resources"
) else (
    echo "Dir %output_testing% has not been created."
)

pause

:SymlinkData
SET desDir=%~1
SET linkDir=%~2

echo The value of parameter 1 is %desDir%
echo The value of parameter 2 is %linkDir%

if exist "%desDir%" (
    rmdir "%desDir%"
)
mklink /D "%desDir%" "%linkDir%"
EXIT /B 0