call Generate_Projects_vs2019.bat

call Build_Solution.bat ../../InsightReflectTool.sln vsany Build Debug win64
call Build_Solution.bat ../../InsightReflectTool.sln vsany Build Release win64

call Build_Solution.bat ../../InsightTools.sln vsany Build Debug win64
call Build_Solution.bat ../../InsightTools.sln vsany Build Release win64

pause