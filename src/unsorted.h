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

// The x and y of the cubic Hermite curve from p0 to p1, with tangents
// m0 and m1, at `time` out of `duration`, as th08's HermiteInterpolate
// does for one coordinate (0x42af80).
void HermiteInterpolate(D3DXVECTOR3 *out, D3DXVECTOR3 *p1, D3DXVECTOR3 *p0, D3DXVECTOR3 *m1, D3DXVECTOR3 *m0,
                        float time, float duration);

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

class AnmLoaded;
class MidiPlayer;
class Sprite;

// A piece of music.
struct BgmInfo
{
    int id;
    // The MIDI file; the wave file has the same name.
    char *path;
    int unknown08;
    int unknown0c;
};
// The game's music, at 0x4a21d0.
extern BgmInfo g_BgmInfos[19];

// Plays the sound effects and music, as th08's (at 0x4dc698).
class SoundPlayer
{
public:
    // Queues a command for the sound thread (0x43ee40).
    void QueueSoundCommand(int type, int arg, char *path);
};
extern SoundPlayer g_SoundPlayer;

// A time kept in the play statistics, as th08's plst fields.
struct PlayTime
{
    DWORD hours;
    DWORD minutes;
    DWORD seconds;
    DWORD milliseconds;
};
// How long the game has run (0x4a818c), and played (0x4a819c).
extern PlayTime g_RunTime;
extern PlayTime g_PlayTime;

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
    // Looks `id` up in g_BgmInfos and loads its MIDI file into slot `id`,
    // or queues the wave file of the same name, as th08's LoadMidi.
    // Returns 0 for MIDI, else 1 (0x42fc20).
    int LoadBgm(int id);
    // Plays the music loaded last. Returns 0 (0x42fcf0).
    int PlayBgm();
    // Stops the music, as th08's StopMusic. Returns 0, or -1 without
    // music (0x42fd30).
    int StopBgm();
    // Looks `id` up in g_BgmInfos and frees what LoadBgm loaded. Returns
    // 0, 1 for an unknown id, or -1 without music (0x42fd90).
    int UnloadBgm(int id);
    // Fades the music out over `seconds`, as th08's FadeOutMusic; the
    // wave fade is stretched while the game runs slow. Returns 0, or -1
    // without music (0x42fe20).
    int FadeOutBgm(float seconds);
    // Advances `frames` by one frame, or by gameSpeed in `subFrame`'s
    // fractions while the game runs slow (0x42f4f0).
    void TickTimer(int *frames, float *subFrame);
    // Starts the three loading sprites at `pos` unless they show, as
    // th08's (0x4302b0).
    void StartLoadingSprites(D3DXVECTOR3 *pos);
    // Interrupts the loading sprites with 1 if they show, and finishes
    // g_LoadingScreenEffect, as th08's (0x430360).
    void StopLoadingSprites();
    // StartLoadingSprites, then has the screen copied to surface 8, as
    // th08's (0x4303c0).
    void StartLoadingSpritesOverScreen(D3DXVECTOR3 *pos);
    // Like StopLoadingSprites, with interrupt 2 and loadingState 2, as
    // th08's (0x4304a0).
    void FinishLoadingSprites();
    // Starts g_LoadingScreenEffect, a ScreenEffect of type `type` + 5,
    // unless one runs, as th08's (0x430500).
    void StartLoadingScreenEffect(int type);
    // Initializes and deletes criticalSections (0x430530, 0x430560).
    void InitCriticalSections();
    void DeleteCriticalSections();
    // Flushes the sprites and turns fog on unless fogState is 1 already,
    // as th08's; returns SetRenderState's result, or 0 (0x42fec0).
    int EnableFog();
    // The same, turning fog off if fogState is set (0x42ff00).
    int DisableFog();
    // Flushes the sprites and sets render state `state` to `value`
    // (0x42ff40).
    int SetRenderState(D3DRENDERSTATETYPE state, DWORD value);
    // Adds the time since the last call to g_RunTime, as th08's
    // (0x42ff70).
    void AddPlayTime();
    // The same for g_PlayTime, restarting from 0 when the clock wrapped
    // (0x430060).
    void AddTotalPlayTime();
    // Returns 0 if verFile lists `version` with `size` and `checksum`,
    // or if `version` is "debug", and -1 otherwise, as th08's
    // (0x430150).
    int CheckVersionFile(char *version, DWORD size, DWORD checksum);
    // Asks the sub thread to close (closeSubThread) and waits for it,
    // as th08's (0x430230).
    void CloseSubThread();

    unsigned char pad_0[0x8];
    IDirect3DDevice8 *d3dDevice;
    unsigned char pad_c[0x436 - 0xc];
    // 0 for no music, 1 to play waves, 2 MIDI.
    BYTE musicMode;
    unsigned char pad_437[0x439 - 0x437];
    // Whether the game runs in a window.
    BYTE windowed;
    unsigned char pad_43a[0x450 - 0x43a];
    GameConfigFlags flags;
    unsigned char pad_454[0x5b8 - 0x454];
    // Below 1 when the game runs slow.
    float gameSpeed;
    MidiPlayer *midi;
    unsigned char pad_5c0[0x5d0 - 0x5c0];
    // The ANM file of the loading sprites.
    AnmLoaded *loadingAnm;
    DWORD stateFlags;
    unsigned char pad_5d8[0x6ac - 0x5d8];
    // The loading thread, as th08's.
    HANDLE subThread;
    DWORD subThreadId;
    // Set to ask the sub thread to end.
    int closeSubThread;
    unsigned char pad_6b8[0x6c0 - 0x6b8];
    CRITICAL_SECTION criticalSections[5];
    // How deep each of criticalSections is entered.
    BYTE criticalSectionDepths[5];
    unsigned char pad_73d[0x740 - 0x73d];
    // 0 while the loading sprites are hidden, 1 while they show, 2 once
    // they are finishing.
    int loadingState;
    unsigned char pad_744[0x794 - 0x744];
    // Whether fog is on, as EnableFog and DisableFog last set it.
    DWORD fogState;
    // timeGetTime() when AddPlayTime and AddTotalPlayTime last ran.
    DWORD lastPlayTimeTick;
    DWORD lastTotalPlayTimeTick;
    unsigned char pad_7a0[0x7a8 - 0x7a0];
    // The contents of the version file, as th08's: lines of a version,
    // a size and a checksum.
    DWORD verFileSize;
    char *verFile;
};
extern App g_App;

// The loading sprites, at 0x4b38b0.
extern Sprite g_LoadingSprites[3];
class ScreenEffect;
// The loading screen's fade, at 0x4b30f8.
extern ScreenEffect *g_LoadingScreenEffect;
