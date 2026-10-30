// Classes and globals not yet placed in their own files.

#pragma once

#include <windows.h>
#include <math.h>

#include "common.h"

// math.h's inline cosf, a single x87 instruction under /Oi, emitted
// out of line under /Ob0 (0x401060). sinf and sqrtf follow it. Named
// wrappers for now: math.h's copies are only emitted where code calls
// them.
float Cos(float angle);
float Sin(float angle);
float Sqrt(float x);
// More of them: atan2f (0x405700), fmodf through _CIfmod (0x405710),
// fabsf (0x40f1c0), tanf (0x42e900) and atanf (0x436ab0).
float Atan2(float y, float x);
float Fmod(float x, float y);
float Fabs(float x);
float Tan(float angle);
float Atan(float x);

// A 2D vector, as th08's.
struct Vector2
{
    // Sets x and y to `radius` along `angle`, with one fsincos (0x441890).
    void SetPolar(float angle, float radius);

    float x;
    float y;
};

// The config's option bits, as th08's GameConfigFlags but in th09's
// order: LoadConfig (0x42f7c0) logs them in this order.
struct GameConfigFlags
{
    unsigned int force16BitTextures : 1;
    unsigned int referenceRasterizer : 1;
    unsigned int noFog : 1;
    unsigned int noDirectInput : 1;
    unsigned int preloadBgm : 1;
    unsigned int noVsync : 1;
    // Text rendering's environment is not detected automatically.
    unsigned int noTextRenderDetect : 1;
    unsigned int unusedBits : 25;
};

// The object that runs the game (th08's App), at 0x4b3100.
struct App
{
    // The config's options, as th08's getters (0x401460).
    int GetNoFog();
    // Advances `frames` by one frame, or by gameSpeed in `subFrame`'s
    // fractions while the game runs slow (0x42f4f0).
    void TickTimer(int *frames, float *subFrame);

    unsigned char pad_0[0x450];
    GameConfigFlags flags;
    unsigned char pad_454[0x5b8 - 0x454];
    // Below 1 when the game runs slow.
    float gameSpeed;
    unsigned char pad_5bc[0x5d4 - 0x5bc];
    DWORD stateFlags;
};
extern App g_App;
