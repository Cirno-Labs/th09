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
    // The draw callbacks, as th08's: the screen after flushing and
    // setting view 2, the whole screen, the play area, and the play area
    // in flashColor (0x422b70, 0x422c10, 0x422bd0, 0x422cd0). Return 1.
    static int OnDrawScreen(ScreenEffect *effect);
    static int OnDrawPlayArea(ScreenEffect *effect);
    static int OnDrawWholeScreen(ScreenEffect *effect);
    static int OnDrawFlash(ScreenEffect *effect);
    // Has the loading screen's fade end: sets finishing and restarts the
    // timer at 0 (0x422930).
    void Finish();

    int unknown00;
    int type;
    ChainElem *calcElem;
    ChainElem *drawElem;
    unsigned char pad_10[0x14 - 0x10];
    // The fade's alpha and the color drawn, as th08's.
    int alpha;
    int duration;
    DWORD color;
    // The flash's color, alpha in the top byte, as th08's.
    DWORD flashColor;
    unsigned char pad_24[0x28 - 0x24];
    // Set by Finish.
    int finishing;
    Timer timer;
};
ASSERT_SIZE(ScreenEffect, 0x38);

// An effect LoadingThread finishes once the title has loaded
// (0x4a7c80).
extern ScreenEffect *g_Unknown4a7c80;

// Clears the back buffer to `color`, as th08's (0x422200).
void ClearScreen(DWORD color);

// Flushes the sprite engine, sets view 2 and clears the screen to
// `color`, as th08's (0x422290).
void ClearViewport(DWORD color);

// Fills `rect` (left, top, right, bottom) with `color`, as th08's
// (0x422960).
void DrawColorRect(float *rect, DWORD color);
