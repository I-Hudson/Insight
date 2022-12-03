:: Download pix
set PIX_URL=https://www.nuget.org/api/v2/package/WinPixEventRuntime/1.0.220810001
set PIX_ZIP=F:\Users\Documents\SourceControl\Github\C++Porjects\Insight\vendor\winpixeventruntime.zip
powershell -Command "Invoke-WebRequest %PIX_URL% -OutFile %PIX_ZIP%"
call ../Tools/UnZip.bat "F:\Users\Documents\SourceControl\Github\C++Porjects\Insight\vendor\winpixeventruntime" "%PIX_ZIP%"
del /f /q "%PIX_ZIP%"


PAUSE