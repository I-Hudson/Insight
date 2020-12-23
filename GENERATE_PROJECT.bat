call vendor\premake\premake5.exe vs2019

@echo OFF
setlocal

if not exist "%cd%\Insight\vendor\assimp\BINARIES" (
    cd "%cd%\Insight\vendor\assimp"
    call "BUILDBINARIES_EXAMPLE.bat"
)

echo "Creating symlinks"

CALL :SymlinkData "%cd%\bin\Debug-windows-x86_64\Sandbox\data", "%cd%\PBC\data"
CALL :SymlinkData "%cd%\bin\Debug-windows-x86_64\InsightEditor\data", "%cd%\PBC\data"
CALL :SymlinkData "%cd%\bin\Release-windows-x86_64\Sandbox\data", "%cd%\PBC\data"
CALL :SymlinkData "%cd%\bin\Dist-windows-x86_64\Sandbox\data", "%cd%\PBC\data"

PAUSE
EXIT /B %ERRORLEVEL% 

:SymlinkData
SET desDir=%~1
SET linkDir=%~2

echo The value of parameter 1 is %desDir%
echo The value of parameter 2 is %linkDir%

if exist "%desDir%" (
    rmdir "%desDir%"
)
mklink /D "%desDir%" "%linkDir%"
EXIT /B 0