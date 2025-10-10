@echo off

rem Clean our root repro
git clean -fxd

rem Clean all submodules
git submodule foreach --recursive git clean -xfd