@echo off

set InsightReflectToolExe=%~dp0..\..\bin\Release-windows-x86_64\InsightReflectTool\InsightReflectTool.exe

call GENERATE_PROJECT.bat premake5-insight_reflect_tool.lua vsany
rem call RunInsightReflectTool.bat

call GENERATE_PROJECT.bat premake5.lua vsany
call GENERATE_PROJECT.bat premake5-standalone.lua vsany

call GENERATE_PROJECT.bat premake5-insight_tools.lua vsany

call Generate_Symlinks.bat
pause