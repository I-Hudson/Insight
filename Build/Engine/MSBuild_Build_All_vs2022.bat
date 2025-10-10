call Generate_Projects_vs2019.bat

call MSBuild_Dependencies_Build_vs2022.bat

call Build_Solution.bat ../../Insight.sln vs2019 Build Debug win64
call Build_Solution.bat ../../Insight.sln vs2019 Build Release win64

call Build_Solution.bat ../../InsightStandalone.sln vs2019 Build Debug win64
call Build_Solution.bat ../../InsightStandalone.sln vs2019 Build Release win64

pause