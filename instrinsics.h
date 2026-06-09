#pragma once

#include <intrin.h>
#include "types.h"

inline ulong Multiply64(unsigned int a, unsigned int b)
{
    return __emulu(a, b);
}
