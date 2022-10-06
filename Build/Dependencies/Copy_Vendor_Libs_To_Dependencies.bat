call :COPY_TO_RELEASE_DEPS "..\..\vendor\oneTBB\oneapi-tbb-2021.6.0\lib\intel64\vc14\tbb.lib"						"lib"
call :COPY_TO_RELEASE_DEPS "..\..\vendor\oneTBB\oneapi-tbb-2021.6.0\lib\intel64\vc14\tbb12.lib"						"lib"
call :COPY_TO_RELEASE_DEPS "..\..\vendor\oneTBB\oneapi-tbb-2021.6.0\lib\intel64\vc14\tbbbind.lib"					"lib"
call :COPY_TO_RELEASE_DEPS "..\..\vendor\oneTBB\oneapi-tbb-2021.6.0\lib\intel64\vc14\tbbbind_2_0.lib"				"lib"
call :COPY_TO_RELEASE_DEPS "..\..\vendor\oneTBB\oneapi-tbb-2021.6.0\lib\intel64\vc14\tbbbind_2_5.lib"				"lib"
call :COPY_TO_RELEASE_DEPS "..\..\vendor\oneTBB\oneapi-tbb-2021.6.0\lib\intel64\vc14\tbbmalloc.lib"					"lib"
call :COPY_TO_RELEASE_DEPS "..\..\vendor\oneTBB\oneapi-tbb-2021.6.0\lib\intel64\vc14\tbbmalloc_proxy.lib"			"lib"

call :COPY_TO_RELEASE_DEPS "..\..\vendor\oneTBB\oneapi-tbb-2021.6.0\redist\intel64\vc14\tbb.dll"					"dll"
call :COPY_TO_RELEASE_DEPS "..\..\vendor\oneTBB\oneapi-tbb-2021.6.0\redist\intel64\vc14\tbb12.dll"					"dll"
call :COPY_TO_RELEASE_DEPS "..\..\vendor\oneTBB\oneapi-tbb-2021.6.0\redist\intel64\vc14\tbbbind.dll"				"dll"
call :COPY_TO_RELEASE_DEPS "..\..\vendor\oneTBB\oneapi-tbb-2021.6.0\redist\intel64\vc14\tbbbind_2_0.dll"			"dll"
call :COPY_TO_RELEASE_DEPS "..\..\vendor\oneTBB\oneapi-tbb-2021.6.0\redist\intel64\vc14\tbbbind_2_5.dll"			"dll"
call :COPY_TO_RELEASE_DEPS "..\..\vendor\oneTBB\oneapi-tbb-2021.6.0\redist\intel64\vc14\tbbmalloc.dll"				"dll"
call :COPY_TO_RELEASE_DEPS "..\..\vendor\oneTBB\oneapi-tbb-2021.6.0\redist\intel64\vc14\tbbmalloc_proxy.dll"		"dll"



call :COPY_TO_DEBUG_DEPS "..\..\vendor\oneTBB\oneapi-tbb-2021.6.0\lib\intel64\vc14\tbb_debug.lib"					"lib"
call :COPY_TO_DEBUG_DEPS "..\..\vendor\oneTBB\oneapi-tbb-2021.6.0\lib\intel64\vc14\tbb12_debug.lib"					"lib"
call :COPY_TO_DEBUG_DEPS "..\..\vendor\oneTBB\oneapi-tbb-2021.6.0\lib\intel64\vc14\tbbbind_2_0_debug.lib"			"lib"
call :COPY_TO_DEBUG_DEPS "..\..\vendor\oneTBB\oneapi-tbb-2021.6.0\lib\intel64\vc14\tbbbind_2_5_debug.lib"			"lib"
call :COPY_TO_DEBUG_DEPS "..\..\vendor\oneTBB\oneapi-tbb-2021.6.0\lib\intel64\vc14\tbbbind_debug.lib"				"lib"
call :COPY_TO_DEBUG_DEPS "..\..\vendor\oneTBB\oneapi-tbb-2021.6.0\lib\intel64\vc14\tbbmalloc_debug.lib"				"lib"
call :COPY_TO_DEBUG_DEPS "..\..\vendor\oneTBB\oneapi-tbb-2021.6.0\lib\intel64\vc14\tbbmalloc_proxy_debug.lib"		"lib"

call :COPY_TO_DEBUG_DEPS "..\..\vendor\oneTBB\oneapi-tbb-2021.6.0\redist\intel64\vc14\tbb_debug.dll"				"dll"
call :COPY_TO_DEBUG_DEPS "..\..\vendor\oneTBB\oneapi-tbb-2021.6.0\redist\intel64\vc14\tbb12_debug.dll"				"dll"
call :COPY_TO_DEBUG_DEPS "..\..\vendor\oneTBB\oneapi-tbb-2021.6.0\redist\intel64\vc14\tbbbind_2_0_debug.dll"		"dll"
call :COPY_TO_DEBUG_DEPS "..\..\vendor\oneTBB\oneapi-tbb-2021.6.0\redist\intel64\vc14\tbbbind_2_5_debug.dll"		"dll"
call :COPY_TO_DEBUG_DEPS "..\..\vendor\oneTBB\oneapi-tbb-2021.6.0\redist\intel64\vc14\tbbbind_debug.dll"			"dll"
call :COPY_TO_DEBUG_DEPS "..\..\vendor\oneTBB\oneapi-tbb-2021.6.0\redist\intel64\vc14\tbbmalloc_debug.dll"			"dll"
call :COPY_TO_DEBUG_DEPS "..\..\vendor\oneTBB\oneapi-tbb-2021.6.0\redist\intel64\vc14\tbbmalloc_proxy_debug.dll"	"dll"

pause

:COPY_TO_DEBUG_DEPS
set lib_file="%cd%\%~1"
set output="%cd%\..\..\deps\Debug-windows-x86_64\%~2\"
xcopy /Y /B %lib_file% %output%

:COPY_TO_RELEASE_DEPS
set output="%cd%\..\..\deps\Release-windows-x86_64\%~2\"
set lib_file="%cd%\%~1"
xcopy /Y /B %lib_file% %output%
