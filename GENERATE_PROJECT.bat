call vendor\premake\premake5.exe vs2019

if not exist "%cd%\Insight\vendor\assimp\BINARIES" (
    cd "%cd%\Insight\vendor\assimp"
    call "BUILDBINARIES_EXAMPLE.bat"
)
PAUSE