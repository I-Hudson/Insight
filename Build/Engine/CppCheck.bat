call "../../vendor/cppcheck-2.9/bin/cppcheck.exe" --force "../../Engine/Core" "../../Engine/Maths" "../../Engine/Input" "../../Engine/Graphics" "../../Engine/Runtime"
echo %errorlevel%
PAUSE