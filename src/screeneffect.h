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
    // its chain elements, as th08's, with `side` new in th09, and type 6
    // drawn over both play areas. Returns it, or NULL (0x422d20).
    static ScreenEffect *StartEffect(int type, int duration, int a, int b, int c, int drawPriority, int side);
    // The update element's added callback, as th08's: restarts the
    // timer. Returns 0 (0x4228f0).
    static int AddedCallback(ScreenEffect *effect);
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
    // Type 3's update callback, as th08's: fades alpha from flashColor's
    // alpha to 0 over duration frames, flashes times over. Returns 0
    // when done, else 1 (0x4225b0).
    static int OnUpdateFlash(ScreenEffect *effect);
    // Type 1's update callback, as th08's: shakes side's camera at
    // random by from shakeFrom to shakeTo pixels over duration frames,
    // holding still while the side's flag0 or GameManager's freeze bits
    // are set. Returns 0 when done, else 1 (0x422660).
    static int OnUpdateShake(ScreenEffect *effect);
    // Type 7's update callback, as th08's: shakes the sprite engine by up
    // to duration pixels, rising, holding and falling (0x4227a0).
    static int OnUpdateShakeRamp(ScreenEffect *effect);
    // The draw callbacks, as th08's: the screen after flushing and
    // setting view 2, the whole screen, the play area, and the play area
    // in flashColor (0x422b70, 0x422c10, 0x422bd0, 0x422cd0). Return 1.
    static int OnDrawScreen(ScreenEffect *effect);
    static int OnDrawPlayArea(ScreenEffect *effect);
    static int OnDrawWholeScreen(ScreenEffect *effect);
    static int OnDrawFlash(ScreenEffect *effect);
    // Type 6's draw callback, new in th09: both sides' play areas in
    // color at alpha (0x422c50). Returns 1.
    static int OnDrawPlayAreas(ScreenEffect *effect);
    // Has the loading screen's fade end: sets finishing and restarts the
    // timer at 0 (0x422930).
    void Finish();

    // The side whose camera shakes.
    int side;
    int type;
    ChainElem *calcElem;
    ChainElem *drawElem;
    unsigned char pad_10[0x14 - 0x10];
    // The fade's alpha and the color drawn, as th08's.
    int alpha;
    int duration;
    union
    {
        DWORD color;
        // Shakes: how far the play area shakes at the start.
        int shakeFrom;
        // Type 7: the frames the shake rises.
        int riseFrames;
        // Type 3: how many more times the color flashes.
        int flashes;
    };
    union
    {
        // The flash's color, alpha in the top byte, as th08's.
        DWORD flashColor;
        // Shakes: how far the play area shakes at the end.
        int shakeTo;
        // Type 7: the frames it holds.
        DWORD holdFrames;
    };
    union
    {
        int flashCount;
        // Type 7: the frames it falls.
        DWORD fallFrames;
    };
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
