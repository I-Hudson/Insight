call Generate_Projects_vs2019.bat

call Build_Solution.bat ../../InsightReflectTool.sln vsany Rebuild Debug win64
call Build_Solution.bat ../../InsightReflectTool.sln vsany Rebuild Release win64

call Build_Solution.bat ../../InsightTools.sln vsany Rebuild Debug win64
call Build_Solution.bat ../../InsightTools.sln vsany Rebuild Release win64

call Build_Solution.bat ../../Insight.sln vsany Rebuild Debug win64
call Build_Solution.bat ../../Insight.sln vsany Rebuild Release win64

call Build_Solution.bat ../../InsightStandalone.sln vsany Rebuild Debug win64
call Build_Solution.bat ../../InsightStandalone.sln vsany Rebuild Release win64

pause