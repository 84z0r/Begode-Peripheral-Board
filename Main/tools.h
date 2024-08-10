#pragma once
#include <inttypes.h>
#include <stddef.h>

namespace Tools
{
    uint8_t* FindPattern(const uint8_t* pStart, size_t searchSize, const uint8_t* pPattern, size_t patternSize, const uint8_t* mask);
    uint8_t* FindPatternLast(const uint8_t* pStart, size_t searchSize, const uint8_t* pPattern, size_t patternSize, const uint8_t* mask);
    template<typename T> T byteSwap(T data)
    {
        if (sizeof(T) == 2)
            return static_cast<T>(__builtin_bswap16(data));
        else if (sizeof(T) == 4)
            return static_cast<T>(__builtin_bswap32(data));
        else if (sizeof(T) == 8)
            return static_cast<T>(__builtin_bswap64(data));

        return data;
    }
}