call "../../vendor/cppcheck/bin/cppcheck.exe" --force "../../Engine/Core" "../../Engine/Maths" "../../Engine/Input" "../../Engine/Graphics" "../../Engine/Runtime"
echo %errorlevel%
PAUSE