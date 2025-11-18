call Generate_Dependencies_Solution.bat

call "../Engine/Build_Solution.bat" "%dir%/../../Engine/Vendor/Dependencies.sln" vsany Build Debug win64
