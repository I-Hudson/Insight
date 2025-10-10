call Generate_Projects_vs2019.bat

call Build_Solution.bat ../../InsightReflectTool.sln vs2019 Build Debug win64
call Build_Solution.bat ../../InsightReflectTool.sln vs2019 Build Release win64

call Build_Solution.bat ../../InsightTools.sln vs2019 Build Debug win64
call Build_Solution.bat ../../InsightTools.sln vs2019 Build Release win64

pause