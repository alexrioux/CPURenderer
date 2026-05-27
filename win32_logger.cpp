// TODO -   see: https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilea
//          for future logging features

#include "logger.h"

#ifdef DEBUG
#ifdef WIN32
#include <windows.h>
#include "types.h"

static HANDLE g_win32StdOutHandle   = 0;
static HANDLE g_win32StdErrHandle   = 0;

static void Win32_Utf8ToUtf16(const void* data, wchar_t* dest);
static void Win32_PrintDbgErrMsg(DWORD errorCode);
static void Win32_Log(HANDLE stdHandle, const char* str, ulong size, WORD colorAttribute);
static ulong StringLength(const char* str);
void InitLogger( );
void PlatformLogInfo(const char* str, ulong size);
void PlatformLogWarning(const char* str, ulong size);
void PlatformLogError(const char* str, ulong size);

void InitLogger( )
{
    // GET CURRENT HANDLES
    g_win32StdOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    g_win32StdErrHandle = GetStdHandle(STD_ERROR_HANDLE);

    if ((!g_win32StdOutHandle || !g_win32StdErrHandle) && AllocConsole( ))
    {
        SetConsoleOutputCP(CP_UTF8);
        g_win32StdOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        g_win32StdErrHandle = GetStdHandle(STD_ERROR_HANDLE);
        if (!g_win32StdOutHandle || !g_win32StdErrHandle)
        {
            OutputDebugStringW(L"win32_logger: At least one console output handle is null");
        }
        else if (g_win32StdOutHandle == INVALID_HANDLE_VALUE || g_win32StdErrHandle == INVALID_HANDLE_VALUE)
        {
            OutputDebugStringW(L"win32_logger: Failed to get console handles -> ");
            Win32_PrintDbgErrMsg(GetLastError( ));
        }
    }
}
void PlatformLogInfo(const char* str, ulong size)
{
    Win32_Log(g_win32StdOutHandle, str, size, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
}
void PlatformLogWarning(const char* str, ulong size)
{
    Win32_Log(g_win32StdOutHandle, str, size, FOREGROUND_GREEN | FOREGROUND_RED);
}
void PlatformLogError(const char* str, ulong size)
{
    // TODO - Maybe pipe this to a file?
    Win32_Log(g_win32StdErrHandle, str, size, FOREGROUND_RED);
}

static void Win32_Log(HANDLE outputHandle, const char* str, ulong size, WORD colorAttribute)
{
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo = {};
    GetConsoleScreenBufferInfo(outputHandle, &consoleInfo);
    SetConsoleTextAttribute(outputHandle, consoleInfo.wAttributes);
    SetConsoleTextAttribute(outputHandle, colorAttribute);

    DWORD charsWritten = 0;
    if (!WriteFile(outputHandle, str, (DWORD)size, &charsWritten, 0))
    {
        OutputDebugStringW(L"win32_logger: Failed to log message -> ");
        Win32_PrintDbgErrMsg(GetLastError( ));
    }

    SetConsoleTextAttribute(outputHandle, consoleInfo.wAttributes); // Reset to original color
}
static void Win32_PrintDbgErrMsg(DWORD errorCode)
{
    // GENERATE ERROR MESSAGE FROM SYSTEM REGISTER
    wchar_t* msg;
    FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        0,
        errorCode,
        MAKELANGID(LANG_SYSTEM_DEFAULT, LANG_USER_DEFAULT), // TODO - MAKELANGID is deprecated
        (wchar_t*)&msg,
        0,
        0
    );
    OutputDebugStringW(msg);
    LocalFree(msg);
}
static void Win32_Utf8ToUtf16(const void* data, wchar_t* dest)
{
    // TODO - Use SIMD

    unsigned long destIndex = 0;
    unsigned long byteIndex  = 0;
    while (((unsigned char*)data)[byteIndex] != '\0')
    {
        unsigned char c = ((unsigned char*)(data))[byteIndex];
        wchar_t wc      = 0;
        if ((c & 0b11110000) == 0b11110000) // 4 bytes UTF-8
        {
            unsigned int surrogate = 0;

            // PACK 4 SUBSEQUENT BYTES
            surrogate   |= (unsigned int)(c & 0b00000111) << 18;
            c            = ((unsigned char*)(data))[byteIndex + 1];
            surrogate   |= (unsigned int)(c & 0b00111111) << 12;
            c            = ((unsigned char*)(data))[byteIndex + 2];
            surrogate   |= (unsigned int)(c & 0b00111111) << 6;
            c            = ((unsigned char*)(data))[byteIndex + 3];
            surrogate   |= (unsigned int)(c & 0b00111111);

            // SHIFT RANGE TO 0
            surrogate               -= 0x10000; // 0xFFFF + 1 = 0x10000
            // ISOLATE HIGH / LOW 10 BITS & APPEND HEADER
            wchar_t high            = ((surrogate & 0x000FFC00) >> 10) | 0xD800;
            wchar_t low             = (surrogate & 0x000003FF) | 0xDC00;

            dest[destIndex++]       = high;
            dest[destIndex++]       = low;
            byteIndex += 4;
        }
        else if ((c & 0b11100000) == 0b11100000) // 3 bytes UTF-8
        {
            // PACK 3 SUBSEQUENT BYTES
            wc  |= (wchar_t)(c & 0b00001111) << 12;
            c    = ((unsigned char*)(data))[byteIndex + 1];
            wc  |= (wchar_t)(c & 0b00111111) << 6;
            c    = ((unsigned char*)(data))[byteIndex + 2];
            wc  |= (wchar_t)(c & 0b00111111);

            dest[destIndex++] = wc;
            byteIndex += 3;
        }
        else if ((c & 0b11000000) == 0b11000000) // 2 bytes UTF-8
        {
            // PACK 2 SUBSEQUENT BYTES
            wc  |= (wchar_t)(c & 0b00011111) << 6;
            c    = ((unsigned char*)(data))[byteIndex + 1];
            wc  |= (wchar_t)(c & 0b00111111);

            dest[destIndex++] = wc;
            byteIndex += 2;
        }
        else
        {
            // ANSI
            dest[destIndex++] = wc | (wchar_t)c;
            byteIndex++;
        }
    }

    dest[destIndex] = '\0';
}

#endif
#else
void InitLogger( ) {}
void LogInfo(const char* str) {}
void LogWarning(const char* str) {}
void LogError(const char* str) {}
#endif
