// The text drawn with ascii.anm, as th08's AsciiText.

#pragma once

#include "common.h"

// The ASCII text, at 0x4ce458. Only what is known so far.
class AsciiText
{
public:
    // Zeroes the sprites, strings and popups and sets them up again, as
    // th08's (0x4343e0).
    void Initialize();
};
extern AsciiText g_AsciiText;
