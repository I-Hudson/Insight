@echo off

set InsightReflectToolExe=%cd%\..\..\bin\Release-windows-x86_64\InsightReflectTool\InsightReflectTool.exe

call GENERATE_PROJECT.bat premake5-insight_reflect_tool.lua vs2019

if not exist "%InsightReflectToolExe%" (
    echo Building Insight Reflect Tool
    echo:
    call Build_Solution.bat ../../InsightReflectTool.sln vs2019 Build Release win64
)
echo Running Insight Reflect Tool to generate files...
start /d "%cd%\..\..\bin\Release-windows-x86_64\InsightReflectTool" InsightReflectTool.exe
echo:

call GENERATE_PROJECT.bat premake5.lua vs2019
call GENERATE_PROJECT.bat premake5-standalone.lua vs2019
call Generate_Symlinks.bat
pause