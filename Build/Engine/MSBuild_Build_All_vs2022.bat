call Generate_Projects_vs2019.bat

call MSBuild_Dependencies_Build_vs2022.bat

call Build_Solution.bat ../../Insight.sln vsany Build Debug win64
call Build_Solution.bat ../../Insight.sln vsany Build Release win64

call Build_Solution.bat ../../InsightStandalone.sln vsany Build Debug win64
call Build_Solution.bat ../../InsightStandalone.sln vsany Build Release win64

pause