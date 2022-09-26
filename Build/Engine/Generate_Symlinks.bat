echo off
 
set output_debug=%cd%/../../bin\Debug-windows-x86_64/Insight_Editor
set output_release=%cd%/../../bin\Release-windows-x86_64/Insight_Editor
set output_testing=%cd%/../../bin\Testing-windows-x86_64/Insight_Editor
set resource_path=%cd%/../../Resources

if exist "%output_debug%" (
    echo "Creating symlinks debug"
    CALL :SymlinkData "%output_debug%\Resources", "%resource_path%"
) else (
    echo "Dir %output_debug% has not been created."
)

if exist "%output_release%" (
echo "Creating symlinks release"
CALL :SymlinkData "%output_release%\Resources", "%resource_path%"
) else (
    echo "Dir %output_release% has not been created."
)

if exist "%output_testing%" (
echo "Creating symlinks release"
CALL :SymlinkData "%output_testing%\Resources", "%resource_path%"
) else (
    echo "Dir %output_testing% has not been created."
)

:SymlinkData
SET desDir=%~1
SET linkDir=%~2

echo The value of parameter 1 is %desDir%
echo The value of parameter 2 is %linkDir%

if exist "%desDir%" (
    rmdir "%desDir%"
)
mklink /D "%desDir%" "%linkDir%"