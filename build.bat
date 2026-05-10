@echo off
cl /Fe:CPURenderer *.cpp /Zi /WX /link user32.lib Gdi32.lib
pause
