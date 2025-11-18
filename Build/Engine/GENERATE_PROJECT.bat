@echo off
@setlocal enableextensions
@setlocal enabledelayedexpansion
@cd /d "%~dp0"

SET prmakeFile=%1
SET option=%2

IF "%option%" == "" GOTO PrintHelp exit
IF "%option%" == "compile" GOTO Compile exit

if "%option%" == "vsany" (
    echo Finding most recent VS option
    for /f "tokens=1,2 delims=," %%a in (vsDevCmdVersions.txt) do (
        if "!option!" == "vsany" (
            if exist "%%a" (
                SET option=%%b
                echo Found vs dev cmd %%b
            )
        )
    )
)

set premake=%~dp0..\..\vendor\premake\premake5.exe

rem call %premake% --version
echo Generating solution for premake file: '%prmakeFile%' with options '!option!'
call %premake% --file=%prmakeFile% %option%
echo:
call Generate_Symlinks.bat

exit /b

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
echo: