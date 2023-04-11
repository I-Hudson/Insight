call Generate_Projects_vs2019.bat
call Build_Solution.bat ../../Insight.sln vs2019 Rebuild Debug win64
call Build_Solution.bat ../../Insight.sln vs2019 Rebuild Release win64

call Build_Solution.bat ../../InsightStandalone.sln vs2019 Rebuild Debug win64
call Build_Solution.bat ../../InsightStandalone.sln vs2019 Rebuild Release win64

call Build_Solution.bat ../../InsightReflectTool.sln vs2019 Rebuild Debug win64
call Build_Solution.bat ../../InsightReflectTool.sln vs2019 Rebuild Release win64

pause