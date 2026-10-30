// Full-screen effects such as fades, each with its own chain elements,
// as th08's.

#pragma once

#include "chain.h"
#include "common.h"
#include "timer.h"

// One effect (0x38 bytes).
class ScreenEffect
{
public:
    // Creates an effect of `type` that lasts `duration` frames and adds
    // its chain elements, as th08's, with `unknown` new in th09 and stored
    // in unknown00 (0x422d20).
    static ScreenEffect *StartEffect(int type, int duration, int a, int b, int c, int drawPriority, int unknown);
    // The update element's deleted callback, as th08's: removes the draw
    // element and deletes the effect. Returns 0 (0x422900).
    static int DeletedCallback(ScreenEffect *effect);
    // Has the loading screen's fade end: sets finishing and restarts the
    // timer at 0 (0x422930).
    void Finish();

    int unknown00;
    int type;
    ChainElem *calcElem;
    ChainElem *drawElem;
    unsigned char pad_10[0x28 - 0x10];
    // Set by Finish.
    int finishing;
    Timer timer;
};
ASSERT_SIZE(ScreenEffect, 0x38);

// Fills `rect` (left, top, right, bottom) with `color`, as th08's
// (0x422960).
void DrawColorRect(float *rect, DWORD color);
