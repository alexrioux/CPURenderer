@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" && cl /Fe:CPURenderer *.cpp /Zi /WX /link user32.lib Gdi32.lib
pause
