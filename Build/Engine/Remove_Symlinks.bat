echo off
 
set output_debug=%~dp0/../../bin\Debug-windows-x86_64/Insight_Editor
set output_release=%~dp0/../../bin\Release-windows-x86_64/Insight_Editor
set output_testing=%~dp0/../../bin\Testing-windows-x86_64/Insight_Editor
set resource_path=%~dp0/../../Resources

if exist "%output_debug%" (
    echo "Deleting symlinks debug"
    rmdir "%output_debug%\Resources"
)

if exist "%output_release%" (
    echo "Deleting symlinks release"
    rmdir "%output_release%\Resources"
)

if exist "%output_testing%" (
    echo "Deleting symlinks testing"
    rmdir "%output_testing%\Resources"
)