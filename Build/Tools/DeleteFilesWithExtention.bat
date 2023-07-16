@echo off

set /p folderPath="Folder to remove files from:"
set /p extension="Extension:"

echo Folder '%folderPath%'
echo Extension '%extension%'

cd %folderPath%

del /s /q *.%extension%