@echo off
 
set resource_path=%~dp0..\..\Resources

set output_debug=%~dp0..\..\bin\Debug-windows-x86_64\Insight_Editor
set output_release=%~dp0..\..\bin\Release-windows-x86_64\Insight_Editor
set output_testing=%~dp0..\..\bin\Testing-windows-x86_64\Insight_Editor

set output_standalone_debug=%~dp0..\..\bin\Debug-windows-x86_64\InsightStandalone
set output_standalone_release=%~dp0..\..\bin\Release-windows-x86_64\InsightStandalone

CALL :CreateSymlink %output_debug% %resource_path%
CALL :CreateSymlink %output_release% %resource_path%
CALL :CreateSymlink %output_testing% %resource_path%
CALL :CreateSymlink %output_standalone_debug% %resource_path%
CALL :CreateSymlink %output_standalone_release% %resource_path%
echo:

exit /b

:CreateSymlink
set ouputDir=%1
set resourcePath=%2
if "%ouputDir%" neq "" (
    if not exist "%ouputDir%" (
        echo Create directory '%ouputDir%'
        mkdir "%ouputDir%"
    )
)
CALL :SymlinkData "%ouputDir%\Resources", "%resourcePath%"

:SymlinkData
SET desDir=%~1
SET linkDir=%~2
if not exist "%desDir%" (
    echo Creating SymLink from '%linkDir%' to '%desDir%'
    mklink /D "%desDir%" "%linkDir%"
)