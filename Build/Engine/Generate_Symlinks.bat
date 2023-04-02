@echo off
 
set resource_path=%cd%/../../Resources

set output_debug=%cd%/../../bin\Debug-windows-x86_64/Insight_Editor
set output_release=%cd%/../../bin\Release-windows-x86_64/Insight_Editor
set output_testing=%cd%/../../bin\Testing-windows-x86_64/Insight_Editor

set output_standalone_debug=%cd%/../../bin\Debug-windows-x86_64/InsightStandalone
set output_standalone_release=%cd%/../../bin\Release-windows-x86_64/InsightStandalone


CALL :CreateSymlink %output_debug% %resource_path%
CALL :CreateSymlink %output_release% %resource_path%
CALL :CreateSymlink %output_testing% %resource_path%
CALL :CreateSymlink %output_standalone_debug% %resource_path%
CALL :CreateSymlink %output_standalone_release% %resource_path%

:CreateSymlink
set ouputDir=%1
set resourcePath=%2
if not exist "%ouputDir%" (
    mkdir "%ouputDir%"
)
CALL :SymlinkData "%ouputDir%\Resources", "%resourcePath%"

:SymlinkData
SET desDir=%~1
SET linkDir=%~2
if not exist "%desDir%" (
    mklink /D "%desDir%" "%linkDir%"
)