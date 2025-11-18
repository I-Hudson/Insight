@echo off

set InsightReflectToolExe=%~dp0..\..\bin\Release-windows-x86_64\InsightReflectTool\InsightReflectTool.exe

call GENERATE_PROJECT.bat premake5-insight_reflect_tool.lua vs2022
rem call RunInsightReflectTool.bat

call GENERATE_PROJECT.bat premake5.lua vs2022
call GENERATE_PROJECT.bat premake5-standalone.lua vs2022

call GENERATE_PROJECT.bat premake5-insight_tools.lua vs2022

call Generate_Symlinks.bat
pause