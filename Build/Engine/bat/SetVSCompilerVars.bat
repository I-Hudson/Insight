@echo off

set CurrentPath=%~dp0

echo Set VS compiler environment variables
for /f "tokens=1,2,3,4,5 delims=," %%a in (%CurrentPath%\..\vsDevCmdVersions.txt) do (
    if %%a NEQ 0 (
        if exist "%%b" (
            set VS_COMPILER_VERSION="%%c"
            echo Selecting VS compiler version generator '%VS_COMPILER_VERSION%'
            set CMAKE_GENERATOR="%%e"
            echo Selecting CMake generator '%VS_COMPILER_VERSION%'

            goto :RETURN
        ) ELSE (
            echo Couldn't find !%%b!
        )
    )
)
exit /b 0

:RETURN
exit /b 0