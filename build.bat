@echo off
set compilerOptions=/Fe:CPURenderer /Zi /W3 /WX /EHa- /GS-
set linkerOptions=/SUBSYSTEM:WINDOWS /ENTRY:Win32EntryPoint /NODEFAULTLIB /OPT:REF
set libraries=user32.lib Gdi32.lib kernel32.lib
set defines=/DWIN32 /DUNICODE
cl %defines% *.cpp %compilerOptions% /link %linkerOptions% %libraries%

@if %ERRORLEVEL% NEQ 0 (
    echo "Failed to build the application"
    pause
) else (
    exit /b
)
