call Generate_Dependencies_Solution.bat

call "../Engine/Build_Solution.bat" "%cd%/../../Engine/Vendor/Dependencies.sln" vsany Build Debug win64
call "../Engine/Build_Solution.bat" "%cd%/../../Engine/Vendor/Dependencies.sln" vsany Build Release win64
