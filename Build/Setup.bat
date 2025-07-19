echo off

echo Settings up Additional dependencies
call Dependencies\SetupAddtionalDependencies.bat

echo Generating dependencies solution
call Dependencies\Build_Dependencies_Solution.bat

echo Building dependencies
call Dependencies\MSBuild_Dependencies_Build_vs2022.bat

echo Generating Insight Engine solutions
call Engine\Generate_Projects_vs2019.bat

echo Running Insight Reflect Tool
call Engine\RunInsightReflectTool.bat