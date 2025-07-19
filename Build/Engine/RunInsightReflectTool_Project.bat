@echo off

set InsightReflectToolExe=%~dp0..\..\bin\Release-windows-x86_64\InsightReflectTool\InsightReflectTool.exe

if not exist "%InsightReflectToolExe%" (
    echo Building Insight Reflect Tool
    echo:
    call Build_Solution.bat ../../InsightReflectTool.sln vs2019 Build Release win64
)

set /p projectPath="Enter abs path to project root folder: "
if not exist "%projectPath%" (
    echo Path %projectPath% does not exists
    GOTO EOF
)

set generatedpath="%projectPath%/Intermediate/CodeFiles/Generated"
set enginePath="%~dp0..\..\Engine"

echo Engine path %enginePath%

echo Running Insight Reflect Tool to generate files...
cd "%~dp0..\..\bin\Release-windows-x86_64\InsightReflectTool" 
InsightReflectTool.exe Type=Project ReflectPath="%projectPath%" GenerateProjectFileOutputPath="%generatedpath%" ParsePath="%enginePath%"
echo:

:EOF
pause