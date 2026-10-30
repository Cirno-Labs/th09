// Classes and globals not yet placed in their own files.

#pragma once

#include <windows.h>
#include <math.h>

#include "common.h"

// cosf, a single x87 instruction under /Oi, as an out-of-line function
// (0x401060). Sin and Sqrt follow it. Written with the double cos(),
// which /Oi makes intrinsic: math.h's inline cosf() would be a call
// under /Ob0.
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

// The object that runs the game (th08's App), at 0x4b3100.
struct App
{
    // Advances `frames` by one frame, or by gameSpeed in `subFrame`'s
    // fractions while the game runs slow (0x42f4f0).
    void TickTimer(int *frames, float *subFrame);

    unsigned char pad_0[0x5b8];
    // Below 1 when the game runs slow.
    float gameSpeed;
    unsigned char pad_5bc[0x5d4 - 0x5bc];
    DWORD flags;
};
extern App g_App;
