call Generate_Dependencies_Solution.bat

call "../Engine/Build_Solution.bat" "%cd%/../../Engine/Vendor/Dependencies.sln" vsany Rebuild Debug win64
