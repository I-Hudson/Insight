@echo off
@setlocal enableextensions
@cd /d "%~dp0"
call vendor\premake\premake5.exe vs2019
echo Project sucessfully generated

pause