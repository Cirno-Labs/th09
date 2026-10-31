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
    // Type 0's update callback, as th08's: fades alpha from 255 to 0
    // over duration frames. Returns 0 when done, else 1 (0x4222c0).
    static int OnUpdateFadeIn(ScreenEffect *effect);
    // Type 2 and 4's update callback, as th08's: fades alpha from 0 to
    // 255 over duration frames, holding still while the game is paused
    // or in the continue menu. Returns 0 when done, else 1 (0x422470).
    static int OnUpdateFadeOut(ScreenEffect *effect);
    // Type 5 and 6's update callback, as th08's but fading up to 255 and
    // holding there, then down from 128 by 255/8 a frame once Finish is
    // called. Returns 0 when done, else 1 (0x4224f0).
    static int OnUpdateLoadingFade(ScreenEffect *effect);
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

// Draws `count` lines through `vertices` as one strip, with
// DrawColorRect's states. New in th09, for Gui's frames (0x422330).
void DrawColoredLineStrip(struct VertexDiffuseXyzrwh *vertices, int count);

// Fills `rect` (left, top, right, bottom) with `color`, as th08's
// (0x422960).
void DrawColorRect(float *rect, DWORD color);
