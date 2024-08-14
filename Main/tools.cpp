#include "tools.h"

uint8_t* Tools::FindPattern(const uint8_t* pStart, size_t searchSize, const uint8_t* pPattern, size_t patternSize, const uint8_t* mask)
{
    size_t patternPos = 0;
    uint8_t* pFind = nullptr;

    for (size_t i = 0; i < searchSize; ++i)
    {
        if (!mask[patternPos] || (pStart[i] == pPattern[patternPos]))
        {
            if (!pFind)
                pFind = const_cast<uint8_t*>(&pStart[i]);

            ++patternPos;
        }
        else
        {
            if (pFind) i -= patternPos;
            patternPos = 0;
            pFind = nullptr;
        }

        if (patternPos == patternSize)
            return pFind;
    }

    return nullptr;
}

uint8_t* Tools::FindPatternLast(const uint8_t* pStart, size_t searchSize, const uint8_t* pPattern, size_t patternSize, const uint8_t* mask)
{
    int patternPos = patternSize - 1;
    uint8_t* pFind = nullptr;

    for (int i = searchSize - 1; i >= 0; --i)
    {
        if (!mask[patternPos] || (pStart[i] == pPattern[patternPos]))
        {
            if (!pFind)
                pFind = const_cast<uint8_t*>(&pStart[i]);

            --patternPos;
        }
        else
        {
            if (pFind) i += patternSize - patternPos - 1;
            patternPos = patternSize - 1;
            pFind = nullptr;
        }

        if (patternPos < 0)
            return pFind - patternSize + 1;
    }

    return nullptr;
}