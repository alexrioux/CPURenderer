#pragma once

void InitLogger( );
void PlatformLogInfo(const char* str, ulong size);
void PlatformLogWarning(const char* str, ulong size);
void PlatformLogError(const char* str, ulong size);

template <ulong N> inline void LogInfo(const char (&str)[N])
{
    PlatformLogInfo(str, N - 1);
}
template <ulong N> inline void LogWarning(const char (&str)[N])
{
    PlatformLogWarning(str, N - 1);
}
template <ulong N> inline void LogError(const char (&str)[N])
{
    PlatformLogError(str, N - 1);
}
