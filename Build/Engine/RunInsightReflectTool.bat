@echo off

set InsightReflectToolExe=%~dp0..\..\bin\Release-windows-x86_64\InsightReflectTool\InsightReflectTool.exe
echo %InsightReflectToolExe%

if not exist "%InsightReflectToolExe%" (
    echo Building Insight Reflect Tool
    echo:
    call Build_Solution.bat ../../InsightReflectTool.sln vs2019 Build Release win64
)
echo Running Insight Reflect Tool to generate files...
start /b /d "%~dp0..\..\bin\Release-windows-x86_64\InsightReflectTool" InsightReflectTool.exe Type=Engine ReflectPath=../../../Engine/ GenerateProjectFileOutputPath=../../../Engine
pause

echo InsightReflectTool has Finished

call Generate_Projects_vs2019.bat

pause