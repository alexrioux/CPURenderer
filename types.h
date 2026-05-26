#pragma once

#if defined(__x86_64__) || defined(_WIN64) || defined(__aarch64__)
typedef unsigned long long ulong;
#else
typedef unsigned int ulong;
#endif

#define ARRAY_COUNT(x) (sizeof(x) / sizeof( ( (x)[0]) ) )
