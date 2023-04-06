@echo off
@setlocal enableextensions
@cd /d "%~dp0"

SET prmakeFile=%1
SET option=%2

IF "%option%" == "" GOTO PrintHelp
IF "%option%" == "compile" GOTO Compile

set premake=..\..\vendor\premake\premake5.exe

call %premake% --version
call %premake% --file=%prmakeFile% %option%
call Generate_Symlinks.bat
GOTO: END

:PrintHelp
echo.
echo Enter 'GENERATE_PROJECT.bat action' where action is one of the following:
echo.
echo   compile           Will generate make file then compile using the make file.
echo   clean             Remove all binaries and intermediate binaries and project files.
echo   codelite          Generate CodeLite project files
echo   gmake             Generate GNU makefiles for Linux
echo   vs2005            Generate Visual Studio 2005 project files
echo   vs2008            Generate Visual Studio 2008 project files
echo   vs2010            Generate Visual Studio 2010 project files
echo   vs2012            Generate Visual Studio 2012 project files
echo   vs2013            Generate Visual Studio 2013 project files
echo   vs2015            Generate Visual Studio 2015 project files
echo   vs2017            Generate Visual Studio 2017 project files
echo   vs2019            Generate Visual Studio 2019 project files
echo   xcode4            Generate Apple Xcode 4 project files

GOTO: END

:END