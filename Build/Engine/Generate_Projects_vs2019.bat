@echo off

call GENERATE_PROJECT.bat premake5.lua vs2019
call GENERATE_PROJECT.bat premake5-standalone.lua vs2019
call GENERATE_PROJECT.bat premake5-insight_reflect_tool.lua vs2019
call Generate_Symlinks.bat
pause