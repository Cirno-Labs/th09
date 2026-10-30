// Classes and globals not yet placed in their own files.

#pragma once

#include <windows.h>
#include <math.h>
#include <d3dx8.h>

#include "common.h"

// math.h's inline float functions are emitted out of line under /Ob0,
// where code calls them: cosf (0x401060) and sinf (0x401070), a single
// x87 instruction each under /Oi. Until their callers are decompiled,
// the rest are named wrappers: sqrtf (0x401080), as Sqrt.
float Sqrt(float x);
// More of them: atan2f (0x405700), fmodf through _CIfmod (0x405710),
// fabsf (0x40f1c0), tanf (0x42e900) and atanf (0x436ab0).
float Atan2(float y, float x);
float Fmod(float x, float y);
float Fabs(float x);
float Tan(float angle);
float Atan(float x);

// The game's random number generator, as th08's, at 0x4ace0c.
class Rng
{
public:
    // Advances the generator and returns 16 random bits (0x42ae20).
    WORD GetRandomU16();
    // 32 random bits, from two GetRandomU16 calls (0x42ae50).
    DWORD GetRandomU32();
    // A random float from 0 up to 1 (0x42ae70).
    float GetRandomF32ZeroToOne();
    // A random float from -1 up to 1 (0x42aea0).
    float GetRandomF32MinusOneToOne();
    // range != 0 ? GetRandomU32() % range : 0, as th08's inline
    // function; this copy is 0x4048e0.
    DWORD GetRandomU32InRange(DWORD range);
    // A random float from 0 up to `range` (0x404910), and from -`range`
    // up to `range` (0x406200).
    float GetRandomF32InRange(float range);
    float GetRandomF32SignedInRange(float range);

    WORD seed;
    unsigned char pad_2[0x2];
    // How many times the generator was advanced.
    DWORD generated;
};
extern Rng g_Rng;

// The log, as th08's, at 0x4b1020.
class Log
{
public:
    // Empties the buffer and clears hasError (0x42b110).
    Log();

    char buffer[0x2000];
    // Where the next message goes.
    char *cursor;
    char hasError;
};
extern Log g_Log;

// a + b, brought back into -pi..pi in at most 17 steps, as th08's
// (0x42aed0).
float AddNormalizeAngle(float a, float b);
// The sine and cosine of `angle`, with one fsincos (0x412780).
void SinCos(float *sinOut, float *cosOut, float angle);
// Rotates `in`'s x and y by `angle` into `out`'s, as th08's (0x42af40).
void Rotate2D(D3DXVECTOR3 *out, D3DXVECTOR3 *in, float angle);

// A 2D vector, as th08's.
struct Vector2
{
    // Sets x and y to `radius` along `angle`, with one fsincos (0x441890).
    void SetPolar(float angle, float radius);
    // The same with radiusX for x and radiusY for y (0x40d4d0).
    void SetPolarEllipse(float angle, float radiusX, float radiusY);

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
    // Bit 3 of stateFlags, as th08's GetNoScore (0x420180).
    int GetNoScore();
    // The config's options, as th08's getters, out of line (0x401460,
    // 0x42d2b0, 0x42d2c0, 0x42d2d0, 0x42ed30, 0x42ed40, 0x42ed50,
    // 0x42ed60).
    int GetNoFog();
    int GetForce16BitTextures();
    int GetReferenceRasterizer();
    int GetWindowed();
    int GetNoDirectInput();
    int GetPreloadBgm();
    int GetNoVsync();
    int GetNoTextRenderDetect();
    // Enters and leaves criticalSections[idx], counting the depth in
    // criticalSectionDepths, as th08's (0x42b130, 0x42b160).
    void EnterCriticalSection(int idx);
    void LeaveCriticalSection(int idx);
    // Advances `frames` by one frame, or by gameSpeed in `subFrame`'s
    // fractions while the game runs slow (0x42f4f0).
    void TickTimer(int *frames, float *subFrame);
    // Initializes criticalSections (0x430530).
    void InitCriticalSections();

    unsigned char pad_0[0x439];
    // Whether the game runs in a window.
    BYTE windowed;
    unsigned char pad_43a[0x450 - 0x43a];
    GameConfigFlags flags;
    unsigned char pad_454[0x5b8 - 0x454];
    // Below 1 when the game runs slow.
    float gameSpeed;
    unsigned char pad_5bc[0x5d4 - 0x5bc];
    DWORD stateFlags;
    unsigned char pad_5d8[0x6c0 - 0x5d8];
    CRITICAL_SECTION criticalSections[5];
    // How deep each of criticalSections is entered.
    BYTE criticalSectionDepths[5];
};
extern App g_App;
