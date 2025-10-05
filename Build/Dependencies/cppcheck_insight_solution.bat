@echo off

set solution="%cd%/../../Insight.sln"
call "../../vendor/cppcheck-2.9/bin/cppcheck" --project=%solution%

pause