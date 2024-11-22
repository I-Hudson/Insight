@echo off

set InsightReflectToolExe=%cd%\..\..\bin\Release-windows-x86_64\InsightReflectTool\InsightReflectTool.exe

if not exist "%InsightReflectToolExe%" (
    echo Building Insight Reflect Tool
    echo:
    call Build_Solution.bat ../../InsightReflectTool.sln vs2019 Build Release win64
)
echo Running Insight Reflect Tool to generate files...
start /d "%cd%\..\..\bin\Release-windows-x86_64\InsightReflectTool" InsightReflectTool.exe Type=Engine ReflectPath=../../../Engine/ GenerateProjectFileOutputPath=../../../Engine

echo InsightReflectTool has Finished

pause