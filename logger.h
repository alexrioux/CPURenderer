#pragma once

#include "types.h"
#include "instrinsics.h"

void InitLogger( );
void PlatformLogInfo(const char* str, ulong size);
void PlatformLogWarning(const char* str, ulong size);
void PlatformLogError(const char* str, ulong size);

static unsigned int UIntToStr(unsigned int integer, char (&dst)[10])
{
    // TODO - extract 2 digits at a time

    unsigned int bytesProcessed = 0;
    unsigned int scalar         = 3435973837; // 0.1 * (2^35)
    const char map[11]          = "0123456789";
    unsigned int i              = 9;
    do
    {
        // EXTRACT DIGIT
        ulong product               = Multiply64(integer, scalar);
        unsigned int quotient       = (unsigned int)(product >> 35);
        unsigned int digit          = integer - (quotient * 10);
        integer                     = quotient;

        dst[i--]                    = map[digit];
        bytesProcessed++;
    }
    while (integer);

    return bytesProcessed;
}

static unsigned int IntToStr(int integer, char (&dst)[11])
{
    if (0x80000000 & integer) // Negative
    {
        unsigned int digitCount = UIntToStr((~integer) + 1, *(char (*)[10])(&dst[1]));
        dst[10 - digitCount] = '-';

        return digitCount + 1; // +1 for sign
    }
    else // Positive
    {
        return UIntToStr((unsigned int)integer, *(char (*)[10])(&dst[1]));
    }
}

static ulong StringCopy(const char* src, char* dst, ulong bytesToCopy) // Returns bytes written
{
    ulong bytesWritten = 0;
    for (; bytesWritten < bytesToCopy; bytesWritten++)
    {
        *(dst + bytesWritten) = src[bytesWritten];
    }
    return bytesWritten;
}

template<typename T>    static constexpr ulong g_typeLength                 = 0;
template<ulong N>       static constexpr ulong g_typeLength<const char[N]>  = N - 1; // N - 1 to avoid copying '\0'
//template<>              static constexpr ulong g_typeLength<char>           = 1;
template<>              static constexpr ulong g_typeLength<unsigned int>   = 10;
template<>              static constexpr ulong g_typeLength<int>   = 10;

template<ulong N> static inline ulong AppendArg(const char (&strArg)[N], char* dst) // Returns bytes written
{
    ulong bytesWritten = StringCopy(strArg, dst, N - 1); // N - 1 to avoid copying '\0'
    return bytesWritten;
}
static inline ulong AppendArg(unsigned int intArg, char* dst) // Returns bytes written
{
    char strBuffer[10];
    unsigned int digitCount = UIntToStr(intArg, strBuffer);
    ulong bytesWritten      = StringCopy(strBuffer + (10 - digitCount), dst, digitCount); // strBuffer is filled in reverse
    return bytesWritten;
}
static inline ulong AppendArg(int intArg, char* dst) // Returns bytes written
{
    char strBuffer[11];
    unsigned int digitCount = IntToStr(intArg, strBuffer);
    ulong bytesWritten      = StringCopy(strBuffer + (11 - digitCount), dst, digitCount); // strBuffer is filled in reverse
    return bytesWritten;
}

template<typename... ArgT> void LogInfo(ArgT&... arg)
{
    constexpr ulong bufferSize = (g_typeLength<ArgT> +...);
    char strBuffer[bufferSize];

    ulong bytesWritten = 0;
    ((bytesWritten += AppendArg(arg, strBuffer + bytesWritten)), ...);

    PlatformLogInfo(strBuffer, bytesWritten);
}
template<typename... ArgT> void LogWarning(ArgT&... arg)
{
    constexpr ulong bufferSize = (g_typeLength<ArgT> +...);
    char strBuffer[bufferSize];

    ulong bytesWritten = 0;
    ((bytesWritten += AppendArg(arg, strBuffer + bytesWritten)), ...);

    PlatformLogWarning(strBuffer, bufferSize);
}
template<typename... ArgT> void LogError(ArgT&... arg)
{
    constexpr ulong bufferSize = (g_typeLength<ArgT> +...);
    char strBuffer[bufferSize];

    ulong bytesWritten = 0;
    ((bytesWritten += AppendArg(arg, strBuffer + bytesWritten)), ...);

    PlatformLogError(strBuffer, bufferSize);
}
