// Classes and globals not yet placed in their own files.

#pragma once

#include <windows.h>
#include <math.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <d3dx8.h>

#include "common.h"
#include "dsutil.h"

// math.h's inline float functions are emitted out of line under /Ob0,
// where code calls them: cosf (0x401060) and sinf (0x401070), a single
// x87 instruction each under /Oi. Until their callers are decompiled,
// the rest are named wrappers: sqrtf (0x401080), as Sqrt.
float Sqrt(float x);
// More of them: atan2f (0x405700), fmodf through _CIfmod (0x405710),
// fabsf (0x40f1c0), tanf (0x42e900), acosf through _CIacos (0x436aa0)
// and atanf (0x436ab0).
float Atan2(float y, float x);
float Fmod(float x, float y);
float Fabs(float x);
float Tan(float angle);
float Acos(float x);
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
    // range != 0 ? GetRandomU16() % range : 0, as th08's inline
    // function; this copy is 0x4048b0.
    WORD GetRandomU16InRange(WORD range);
    // range != 0 ? GetRandomU32() % range : 0, as th08's inline
    // function; this copy is 0x4048e0.
    DWORD GetRandomU32InRange(DWORD range);
    // A random float from 0 up to `range` (0x404910), and from -`range`
    // up to `range` (0x406200).
    float GetRandomF32InRange(float range);
    float GetRandomF32SignedInRange(float range);

    // Inline; the copy is the GameManager object's (0x41a89e).
    WORD GetSeed();
    // Sets the seed (0x420160).
    void SetSeed(WORD seed);
    // Sets generated to 0 (0x420170).
    void ResetGenerated();

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

    // Empties the buffer, as th08's (0x42d230).
    void Reset();
    // If anything was logged, ends it with a line of dashes, shows it in
    // a message box if hasError is set and writes it to log.txt, as
    // th08's (0x42d240).
    void Flush();

    char buffer[0x2000];
    // Where the next message goes.
    char *cursor;
    char hasError;
};
extern Log g_Log;

// The game's window, as th08's (0x44 bytes, at 0x4b30b0). Only the
// fields known so far.
#pragma pack(push, 4)
struct GameWindow
{
    // The time in seconds, from the performance counter if it has a
    // frequency, else from timeGetTime() (0x42cc90).
    double GetTime();
    // Runs a frame once its time has come, as th08's, with the 2D view
    // of the whole screen. Returns 0, or 1 or 2 to quit, as the calc
    // chain asks (0x42dc40).
    int RunFrame();

    HWND hwnd;
    // Cleared by InitD3dDevice, as th08's.
    int isClosing;
    // WM_ACTIVATEAPP's wParam, and whether it is 0, as th08's.
    int isAppActive;
    int isAppInactive;
    char curFrame;
    unsigned char pad_11[0x3];
    // The performance counter's frequency, as a LARGE_INTEGER but only
    // 4-aligned, as th08's.
    DWORD perfFrequency[2];
    // Set when the game was not started by its own executable or a
    // shortcut to it, as th08's.
    unsigned char forceClockPacing;
    unsigned char pad_1d[0x20 - 0x1d];
    // The screen saver and power settings WinMain turns off and restores,
    // as th08's.
    UINT screenSaveActive;
    UINT lowPowerActive;
    UINT powerOffActive;
    double now;
    double lastTime;
    double nextFrameTime;
};
#pragma pack(pop)
ASSERT_SIZE(GameWindow, 0x44);
extern GameWindow g_GameWindow;
// The single-instance mutex, as th08's.
extern HANDLE g_AppMutex;

// Creates g_App.d3d, as th08's. Returns 0, or logs and returns 1
// (0x42cd10).
int CreateD3d();
// Brings `hwnd` to the front despite the foreground lock, as th08's.
// __stdcall, unlike the rest of the game (0x42d0c0).
void __stdcall ForceForegroundWindow(HWND hwnd);
// The window's procedure, as th08's: WM_ACTIVATEAPP keeps whether
// the game is active, the cursor is hidden while it is in full screen
// and active, WM_CLOSE sets bit 7 of g_App's stateFlags (0x42d3d0).
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
// Creates the single-instance mutex, still th08's "Touhou 08 App", and
// sets forceClockPacing if the game was started by something other
// than its own executable, as th08's. Returns 0, or -1 if the game
// already runs (0x42d900).
int CheckSingleInstance();
// Creates the device and the cameras, and logs the capabilities
// (0x42dd90).
int InitD3dDevice();
// Sets the device's render states up, as th08's (0x42d5f0).
void SetDefaultRenderStates();
// Presents the frame, as th08's: resets the device if it is lost, runs
// the pending copies, saves a screenshot if the menu input just pressed
// 0x800 and bit 10 of g_App's stateFlags is clear, and counts
// redrawFrames down unless the game is paused (0x42db50).
void PresentFrame();
// Sums the executable, as th08's, opening it by its name alone, and
// keeps the sum and the size in g_App. Returns the sum, or -1
// (0x42da90).
int ChecksumExecutable();
// The target of the shortcut `path`, `size` bytes at most, in `out`, as
// th08's. Returns 1, or 0 if it cannot be resolved (0x42d140).
int ResolveShortcut(char *path, char *out, int size);

// Appends a printf-style message to `log` if it fits, inside critical
// section 3, as th08's. Returns `fmt` (0x42c5c0).
int LogPrintf(Log *log, const char *fmt, ...);
// The same with a smaller buffer, setting hasError (0x42c660).
int LogError(Log *log, const char *fmt, ...);

// Logs and returns 1 if neither joystick answers, else reads both
// joysticks' capabilities into g_JoyCaps and returns 0, as th08's for
// one (0x42c8f0).
WORD InitJoystick();
// The joysticks' capabilities, from joyGetDevCaps (0x4ac8d8).
extern JOYCAPSA g_JoyCaps[2];

// Releases every key in the keyboard state, as th08's (0x42ad70).
void ResetKeyboard();
// Opens the game's archive, as th08's. Returns 0, or -1 (0x42f100).
int OpenGameArchive();
// Measures the refresh rate, as th08's. Returns nonzero if it is too
// far from 60 Hz (0x42f1a0).
int MeasureRefreshRate();
// Counts the frame and, every half second, measures the frame rate, as
// th08's; shows it if `draw` is set (0x4316b0).
void DrawFps(int draw);
// Reads player `player`'s keyboard and pad buttons into
// g_PlayerInputs[player] and returns them (0x42b850).
WORD ReadInput(int player);
// Reads the three players' inputs (0x42e9c0).
void ReadInputs();

// The directory the game's loose files are read from (0x4acd08), and
// where GetDataPath builds a path in it (0x4acc00).
extern char g_DataDir[];
extern char g_PathBuffer[0x108];
// `name` in g_DataDir, in g_PathBuffer. __stdcall, unlike the rest of
// the game (0x42adc0).
char *__stdcall GetDataPath(char *name);

// Whether `path` in g_DataDir can be opened, inside critical section
// 2, as th08's (0x42c480).
int FileExists(char *path);

// Writes `size` bytes of `data` to `path` in g_DataDir, inside critical
// section 2, as th08's. Returns 0, -1 if the file cannot be created, or
// -2 if not all was written (0x42c4e0).
int WriteToFile(char *path, void *data, int size);

// Prints to the debug log in th08; empty in this build (0x43d2b0).
void DebugPrint(const char *fmt, ...);

// Decrypts `size` bytes of `data` into a new block, as th08's
// (0x42c180).
BYTE *DecryptData(BYTE *data, int size, BYTE key, BYTE step, int blockSize, int limit);

// Encrypts `size` bytes of `data` into a new block, the reverse of
// DecryptData, as th08's (0x42c370).
BYTE *EncryptData(BYTE *data, int size, BYTE key, BYTE step, int blockSize, int limit);

// The DecryptData parameters for one kind of data file, as th08's.
struct CryptParams
{
    // The fourth byte of the file's header, plus 0x10 times (its index
    // + 1).
    BYTE type;
    BYTE key;
    BYTE step;
    BYTE unusedByte3;
    int blockSize;
    int limit;
};
ASSERT_SIZE(CryptParams, 0xc);
// At 0x4a1e60.
extern CryptParams g_CryptParams[8];
// "edz" plus 0x20, 0x40 and 0x60: the first three bytes of an encrypted
// data file (0x4a1ec0).
extern BYTE g_CryptMagic[];

// If `data` (`fileSize` bytes) starts with the encrypted file header,
// decrypts the rest into a new block, frees `data` and returns the new
// one; else returns `data`, as th08's (0x42c290).
BYTE *DecryptFileData(BYTE *data, int *size, int fileSize);
// Reads the file at `path`, from the archive unless `isExternal` is set,
// else from g_DataDir, decrypted, into a new block, inside critical
// section 2, as th08's LoadFileData. Stores its size in `size` unless
// NULL. Returns the data, or NULL (0x42c970).
void *OpenPath(char *path, int *size, int isExternal);

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

// The buttons each action is mapped to, as pad button numbers; negative
// when unmapped, as th08's.
struct ControllerMapping
{
    short shootButton;
    short bombButton;
    short focusButton;
    short menuButton;
    short upButton;
    short downButton;
    short leftButton;
    short rightButton;
    short skipButton;
};
ASSERT_SIZE(ControllerMapping, 0x12);

// The players' inputs, at 0x4ace18, and the third one the menus read.
struct PlayerInput
{
    // Whether a button of `mask` is down in either of the masks at 4
    // and 6 (0x423158).
    WORD IsPressed(WORD mask)
    {
        return (WORD)(this->unknown6 & mask) != 0 || (WORD)(this->unknown4 & mask) != 0;
    }

    // unknown0 & mask and unknown6 & mask (0x40e2b0, 0x40e2d0), and the
    // same for the second set, unknown2c and unknown32 (0x415cb0,
    // 0x415cd0).
    WORD IsHeld(WORD mask);
    WORD IsJustPressed(WORD mask);
    WORD IsHeld2(WORD mask);
    WORD IsJustPressed2(WORD mask);
    // Clears unknown4 and updates the second set from unknown2c and
    // unknown2e: counts how long each button is held, setting bit 0 of
    // unknown30 every 8 frames from the 26th, and takes the buttons just
    // pressed into unknown32 and just released into unknown34
    // (0x4200f0).
    void UpdateRepeat2();

    WORD unknown0;
    unsigned char pad_2[0x2];
    WORD unknown4;
    WORD unknown6;
    unsigned char pad_8[0x2c - 0x8];
    WORD unknown2c;
    WORD unknown2e;
    WORD unknown30;
    WORD unknown32;
    WORD unknown34;
    unsigned char pad_36[0x38 - 0x36];
    // How long each button of unknown2c has been held.
    WORD heldFrames2[16];
    // The default mappings.
    ControllerMapping mappings[3];
};
ASSERT_SIZE(PlayerInput, 0x8e);
extern PlayerInput g_PlayerInputs[3];

// The record of what has been played, at 0x4a8180, around g_RunTime
// and g_PlayTime (+0xc, +0x1c). Only what is known so far.
struct PlayRecord
{
    // Counts a clear of character `character` on difficulty
    // `difficulty`, up to 999999 (0x40e3a0).
    void AddClear(char character, int difficulty);
    // Opens entry `entry` of the three menus' lists (0x40e3d0, 0x40e3e0,
    // 0x40e3f0).
    void OpenEntry(char entry);
    void OpenEntry2(char entry);
    void OpenEntry3(char entry);
    // Whether character `character` has a clear on any of the five
    // difficulties (0x40e400).
    BOOL HasCleared(char character);
    // Character `character`'s count for difficulty 5 (0x40e450).
    int GetClears5(char character);
    // Counts one more for character `character`'s difficulty 5, up to
    // 999999 (0x4158e0).
    void AddClear5(char character);
    // Character `character`'s count for difficulty `difficulty`, or,
    // when it is negative, those of the first four or'ed together
    // (0x4234a7).
    int GetClearFlags(char character, int difficulty);

    // The score file's "PLST" chunk header: version 3.
    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    BYTE unknown9;
    unsigned char pad_a[0x2c - 0xa];
    // Which pieces of music have been heard, by BgmInfo::unknown0c.
    BYTE bgmHeard[0x20];
    // Which entries of three of the menus' lists are open.
    BYTE openEntries[16];
    BYTE openEntries2[16];
    BYTE openEntries3[16];
    // Each character's clears on each difficulty, to the end of the
    // record.
    int clearFlags[16][6];
};
ASSERT_SIZE(PlayRecord, 0x1fc);
extern PlayRecord g_PlayRecord;

// The header of thbgm.dat, as th08's.
struct BgmDatHeader
{
    // 'ZWAV'.
    DWORD magic;
    int version;
    int gameVersion;
    DWORD unknownC;
};
ASSERT_SIZE(BgmDatHeader, 0x10);

// The config file's contents, in App (th08 kept them in App itself).
struct GameConfig
{
    // Calls SetDefaults. Inline: the title menu's object expands it, and
    // the copy is at 0x41a98b.
    GameConfig()
    {
        this->SetDefaults();
    }
    // Zeroes the config and sets the defaults (0x41a8a2).
    void SetDefaults();

    // Each player's three mappings.
    ControllerMapping mappings[2][3];
    unsigned char pad_6c[0xa4 - 0x6c];
    DWORD version;
    short padThresholdX;
    short padThresholdY;
    BYTE unknownAc;
    BYTE unknownAd;
    // 0 for no music, 1 to play waves, 2 MIDI.
    BYTE musicMode;
    // Whether the sound effects play.
    BYTE playSounds;
    // The difficulty the menu last chose; network play sets 1.
    BYTE difficulty;
    // Whether the game runs in a window.
    BYTE windowed;
    // How many frames to skip drawing, as th08's.
    BYTE frameSkip;
    BYTE unknownB3;
    BYTE unknownB4;
    BYTE unknownB5;
    BYTE unknownB6;
    BYTE unknownB7;
    BYTE unknownB8;
    BYTE unknownB9;
    char bgmVolume;
    char seVolume;
    unsigned char pad_bc[0xc8 - 0xbc];
    GameConfigFlags flags;
};
ASSERT_SIZE(GameConfig, 0xcc);

class AnmLoaded;
class MidiPlayer;
class MidiTimer;
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
// The unknown08 of the piece PlayMusic played last (0x4dc690); the
// gui reads it.
extern int g_BgmUnknown08;

// A command PlayQueuedSounds carries out over several frames, for the
// streaming wave music, as th08's.
struct SoundCommand
{
    // 0 ends the queue.
    int type;
    int arg;
    // How far the command has got.
    int stage;
    char path[0x100];
};
ASSERT_SIZE(SoundCommand, 0x10c);

// An entry of thbgm.fmt: where a track is in thbgm.dat and its format,
// as th08's.
struct BgmFormat
{
    // The track's file name; an empty one ends the list.
    char name[0x10];
    DWORD offset;
    DWORD size;
    // CWaveFile::ResetFile limits the track to `loopEnd` bytes and, when
    // looping, skips its first `loopStart`.
    int loopStart;
    int loopEnd;
    WAVEFORMATEX format;
};
ASSERT_SIZE(BgmFormat, 0x34);

// A sound effect: the loaded file it plays, its volume and its priority.
struct SoundInfo
{
    int buffer;
    short volume;
    short priority;
};
// 0x36 sound effects, at 0x4a1f80 (th08 0x2e).
extern SoundInfo g_SoundInfos[0x36];
// The sound files LoadSounds loads, by index: 0x27 (th08 0x24), at
// 0x4a2130.
extern char *g_SoundPaths[0x27];

// Plays the sound effects and music, as th08's (at 0x4dc698).
class SoundPlayer
{
public:
    // Zeroes everything; no sound is loaded (0x43e470).
    SoundPlayer();
    // Creates the DirectSound objects for `hwnd`. Returns 0, or -1 on
    // failure (0x43e500).
    int Initialize(HWND hwnd);
    // Releases the sounds and DirectSound (0x43eeb0).
    int Release();
    // Stops the wave music: ends the notification thread and releases
    // the stream (0x43ebc0).
    void StopBgm();
    // The index in bgmFormat of the track named as the file name of
    // `path`, or 0 if there is none (0x43e1f0).
    int FindTrackIndex(char *path);
    // Loads the wave file at `path` into soundBuffers[`index`]. Returns
    // 0, also when there is no sound, or -1 on failure (0x43e770).
    int LoadSound(int index, char *path);
    // Loads the track list at `path` (thbgm.fmt) into bgmFormat. Returns
    // 0 or -1 (0x43e2c0).
    int LoadBgmFormat(char *path);
    // Starts streaming the wave music from the file at `path`
    // (thbgm.dat), as bgmFormat's first track, after stopping what
    // plays, and starts the thread that streams it. Returns 0 or -1
    // (0x43efa0).
    int LoadBgm(char *path);
    // Switches the wave music to the track named as the file name of
    // `path`. Returns 0, or -1 when no music is loaded (0x43e9e0).
    int SwitchBgmTrack(char *path);
    // Reads the track named as the file name of `path` out of bgmPath
    // into preload slot `slot`, unless it is there already. Does nothing
    // unless the config's preloadBgm is set. Returns 0 or -1 (0x43ea30).
    int PreloadBgm(int slot, char *path);
    // Starts the wave music from preload slot `slot`, as LoadBgm does,
    // or switches the stream to its track without preloading. Returns 0
    // or -1 (0x43f0a0).
    int PlayPreloadedBgm(int slot);
    // Loads the sound effects ("data/wav/se_plst00.wav", ...) (0x43ec80).
    int LoadSounds();
    // Carries out the commands' next stages and plays the queued sound
    // effects. Returns the first command's type (0x43f1f0).
    int PlayQueuedSounds();
    // Fades the wave music out over `seconds`. An out-of-line copy
    // (0x43e4e0).
    void FadeOutBgm(float seconds)
    {
        if (this->bgm != NULL)
        {
            this->bgm->FadeOut(seconds);
        }
    }
    // Frees preload slot `slot`'s data, if any. An out-of-line copy
    // (0x43e4a0).
    void FreePreloadedBgm(int slot)
    {
        if (this->bgmPreloadData[slot] != NULL)
        {
            g_MemoryManager.Free(this->bgmPreloadData[slot]);
            this->bgmPreloadData[slot] = NULL;
        }
    }
    // Queues sound effect `idx` with `arg` to start on the next update:
    // a sound already queued collects up to 0x80 arguments, and up to 12
    // sounds are queued; th08's QueueSound (0x43e2f0).
    void PlaySoundByIdx(int idx, int arg);
    // PlaySoundByIdx with the pan for horizontal position `x`, as th08's
    // QueueSoundPanned (0x43e380).
    void QueueSoundPanned(int idx, float x);
    // Queues a command of `type` with `arg` and `path` in the first free
    // entry of `commands` (0x43ee40).
    void QueueSoundCommand(int type, int arg, char *path);
    // Queues the "UnPause" command (7). Inline in th08; this is the copy
    // at 0x423498, in an /O1 object.
    void UnPause();
    // Steps the wave music's fades, as th08's (0x42fbe0).
    void UpdateBgmFades();

    IDirectSound8 *dsound;
    DWORD unusedDword004;
    // The loaded sound files' buffers.
    IDirectSoundBuffer *soundBuffers[0x80];
    // Each sound effect's own buffer.
    IDirectSoundBuffer *sounds[0x80];
    // Set when a sound is queued; -1 at first.
    int queuedSound[0x80];
    // A silent buffer, looping from start-up.
    IDirectSoundBuffer *silence;
    // The window the sound timer posts to.
    HWND hwnd;
    // NULL until Initialize succeeds.
    CSoundManager *manager;
    // The thread streaming the wave music and its ID.
    DWORD notifyThreadId;
    HANDLE notifyThread;
    DWORD unusedDword61c;
    // The sound effects to play, or -1.
    int queue[0xc];
    // How many arguments each queued sound has, and the arguments.
    int queueArgCounts[0xc];
    int queueArgs[0xc][0x80];
    // The thbgm.fmt entries of the preloaded tracks, their data, and
    // its size.
    BgmFormat *bgmEntries[0x10];
    void *bgmPreloadData[0x10];
    void *bgmPreloadCursor[0x10];
    DWORD bgmPreloadSize[0x10];
    // The preloaded track playing.
    int bgmPlaying;
    // The contents of thbgm.fmt.
    BgmFormat *bgmFormat;
    // The commands for the wave music, in order.
    SoundCommand commands[0x20];
    // The file names of the preloaded tracks (th08 0x10 of them).
    char bgmNames[0x20][0x100];
    // The file the wave music streams from.
    char bgmPath[0x100];
    // The wave music playing; NULL when stopped.
    CStreamingSound *bgm;
    // Signalled as the wave music plays.
    HANDLE notifyEvent;
    DWORD unusedDword6210;
    // Added to every BgmFormat offset by CWaveFile::ResetFile.
    DWORD bgmFileOffset;
    // The music and sound effect volumes in percent, 100 at first.
    int bgmVolume;
    int seVolume;
    DWORD bgmBufferVolume;
};
ASSERT_SIZE(SoundPlayer, 0x6224);
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

// A record in App's lists.
struct RunRecord
{
    WORD unknown0;
    WORD seed;
    int value;
    int unknown8;
};
ASSERT_SIZE(RunRecord, 0xc);

// The object that runs the game (th08's App), at 0x4b3100.
// A camera of App's views. Only the fields known so far.
struct Camera
{
    // SetupCamera3D looks from offset + eye to at + eye, with `up` up.
    D3DXVECTOR3 eye;
    D3DXVECTOR3 at;
    D3DXVECTOR3 up;
    unsigned char pad_24[0x30 - 0x24];
    // A unit to the camera's right, as th08's Stage camera's.
    D3DXVECTOR3 cameraRight;
    D3DXVECTOR3 offset;
    // The field of view, in radians.
    float fov;
    D3DXMATRIX viewMatrix;
    D3DXMATRIX projectionMatrix;
    D3DVIEWPORT8 viewport;
    // Its index in App::cameras.
    int index;
    // Copied to the sprite engine's shake when the camera is set up.
    float shakeX;
    float shakeY;
};
ASSERT_SIZE(Camera, 0xf0);

struct App
{
    // Sets up view `view`, its camera and viewport: 0 the play area, 2
    // the whole screen. Inline; the copy is the stage's object's
    // (0x401390).
    void SetView(int view)
    {
        this->camera = &this->cameras[view];
        this->SetupCamera3D(this->camera);
        this->d3dDevice->SetViewport(&this->camera->viewport);
        this->view = view;
    }
    // SetView, setting the camera up through SetupCamera. Inline; the
    // copy is at 0x403d90.
    void SetView2D(int view)
    {
        this->camera = &this->cameras[view];
        this->SetupCamera(this->camera);
        this->d3dDevice->SetViewport(&this->camera->viewport);
        this->view = view;
    }
    // Sets up the camera and the projection of `camera` (0x430590).
    void SetupCamera(Camera *camera);
    // Sets up the current camera and its viewport again. Inline; the copy
    // is at 0x4013e0.
    void ResetView()
    {
        this->SetupCamera(this->camera);
        this->d3dDevice->SetViewport(&this->camera->viewport);
    }
    // Sets up the projection of `camera` in 3D, from its eye, target
    // and up vectors (0x4306b0).
    void SetupCamera3D(Camera *camera);
    // Sets up the three cameras looking down from z 1000 with a field of
    // view of pi / 6: the whole screen (2) and the two sides' play areas
    // (0, 1) (0x4307c0).
    void InitCameras();
    // ResetView with SetupCamera3D. Inline; the copy is at 0x401410.
    void ResetView3D()
    {
        this->SetupCamera3D(this->camera);
        this->d3dDevice->SetViewport(&this->camera->viewport);
    }
    // Bit 3 of stateFlags, as th08's GetNoScore (0x420180).
    int GetNoScore();
    // The config's options, as th08's getters, out of line (0x401460,
    // 0x42d2b0, 0x42d2c0, 0x42d2d0, 0x42ed30, 0x42ed40, 0x42ed50,
    // 0x42ed60).
    int GetNoFog();
    int GetForce16BitTextures();
    int GetReferenceRasterizer();
    int GetWindowed();
    int GetNoDirectInput()
    {
        return this->config.flags.noDirectInput;
    }
    int GetPreloadBgm();
    int GetNoVsync();
    int GetNoTextRenderDetect();
    // Saves the back buffer to the bitmap `path` (0x42f540).
    void SaveScreenshot(char *path);
    // Whether subThread is set, as th08's. Inline; the copy the others
    // call is at 0x42ed70.
    BOOL HasSubThread()
    {
        return this->subThread != NULL;
    }
    // Enters and leaves criticalSections[idx], counting the depth in
    // criticalSectionDepths, as th08's (0x42b130, 0x42b160).
    void EnterCriticalSection(int idx);
    void LeaveCriticalSection(int idx);
    // Puts a record of `value` with `unknown` and `seed` in side `side`'s
    // list, before the first greater one, and makes it the side's last
    // (0x42e9e0).
    void AddRecord(int side, int value, WORD unknown, WORD seed);
    // Puts a record of `value` with `seed` and the side's last record's
    // unknown0 in the side's list, before the first greater one, marked
    // with unknown8 1 (0x42eac0).
    void InsertRecord(int side, int value, WORD seed);
    // Takes the side's first record off its list: returns its unknown0,
    // or the side's input if its unknown8 is set, and stores its seed
    // and unknown8 (0x42eb80).
    WORD PopRecord(int side, WORD *seed, int *unknown8);
    // Whether the side has a record and both sides' first records have
    // the same value (0x42ecf0).
    BOOL RecordsMeet(int side);
    // Resets the run: clears the fields from 0x458 and the records,
    // restarts the timers, adds each side a record of -9999 with the
    // random seed, and sets network play's unknownA8 (0x42ec30).
    void ResetRun();
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
    // Plays the piece `id` of the table at 0x4a21d0; what `arg` does is
    // not known. Returns 1 for an unknown id (0x431930).
    int PlayMusic(int id, int arg);
    // Plays the MIDI file `path`, or queues the wave file of the same
    // name, as th08's PlayMusic, and marks piece `heard` heard. Returns
    // 0, or -1 without music (0x431a30).
    int PlayMusicFile(char *path, int heard);
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
    // Reads the config from `path` into config, or sets the defaults if
    // it is missing or wrong, with the music mode from whether
    // thbgm.dat is there, as th08's; logs the options and writes the
    // config back. Returns 0, or -1 if thbgm.dat has the wrong version or
    // the config cannot be written (0x42f7c0).
    int LoadConfig(char *path);
    // Asks the sub thread to close (closeSubThread) and waits for it,
    // as th08's (0x430230).
    void CloseSubThread();
    // Runs `proc` with `arg` in the sub thread, as th08's (0x431ae0).
    int StartSubThread(LPTHREAD_START_ROUTINE proc, LPVOID arg);
    // The chain callbacks, as th08's: the update element's callback and
    // its added and deleted callbacks (0x431110, 0x432090, 0x431b70),
    // and the draw elements' at priorities 0x1f, 0 and 2 (0x431b30,
    // 0x431540, 0x42ed80).
    static int OnUpdate(App *app);
    static int AddedCallback(App *app);
    static int DeletedCallback(App *app);
    static int OnDraw(App *app);
    static int OnDrawLogo(App *app);
    static int OnDrawLoading(App *app);
    // Resets the scenes and adds the update and draw elements, as th08's.
    // Returns 0, or what adding the update element returned (0x432180).
    static int RegisterChain();
    // Loads the game's resources in the sub thread, as th08's (0x431d10).
    static void LoadingThread(App *app);
    // Runs a frame of network play; 1 to run another, 2 to skip the
    // frame, 3 when it ended and 4 when it failed (0x430aa0).
    int UpdateNetPlay();
    // Clears subThread, closeSubThread and unknown6b8 (0x41a997).
    void ResetSubThread();
    // Zeroes unknown77c to unknown790 (0x423382).
    void ClearUnknown77c();

    // WinMain's instance.
    HINSTANCE hInstance;
    // Created by CreateD3d.
    IDirect3D8 *d3d;
    IDirect3DDevice8 *d3dDevice;
    // DirectInput, the keyboard and up to two pads, set up by
    // InitDirectInput.
    IDirectInput8 *dinput;
    IDirectInputDevice8 *keyboard;
    IDirectInputDevice8 *joystick;
    IDirectInputDevice8 *joystick2;
    DIDEVCAPS joystickCaps;
    DIDEVCAPS joystickCaps2;
    // The game's window, as th08's.
    HWND hwnd;
    // The views' cameras.
    Camera cameras[3];
    // The camera of the view SetView last set up.
    Camera *camera;
    // The view SetView last set up.
    int view;
    // The parameters the device is created with, as th08's.
    D3DPRESENT_PARAMETERS presentParams;
    // The 16 ms timer LoadingThread starts if stateFlags bit 6 is set.
    MidiTimer *timer;
    GameConfig config;
    // Counts the frames, as th08's.
    int frames;
    int unknown458;
    int unknown45c;
    int unknown460;
    // The time ResetRun ran.
    DWORD unknown464;
    DWORD unknown468;
    DWORD unknown46c;
    unsigned char pad_470[0x474 - 0x470];
    DWORD unknown474;
    int unknown478;
    // Each side's ten records, ordered by value, and its last one.
    RunRecord records[2][10];
    RunRecord lastRecords[2];
    // Drawn by OnDraw while unknown478 is set.
    Sprite *unknown584;
    unsigned char pad_588[0x58c - 0x588];
    // The scene running, as th08's.
    int curScene;
    // The scene to go to.
    int scene;
    int prevScene;
    // Set when the game ends, unless it goes on to scene 3, 10 or 12:
    // the game's objects then free their resources, as th08's.
    int freesResources;
    // Set when the game restarts, as th08's isRestarting.
    int unknown59c;
    unsigned char pad_5a0[0x5a4 - 0x5a0];
    // How many more frames to redraw, as th08's: set to 2 when the
    // device is lost, cleared by LoadingThread.
    int redrawFrames;
    // Set while the frame rate is not shown, as th08's.
    int hideFps;
    // Set when the config turns vertical sync off.
    int noVsync;
    unsigned char pad_5b0[0x5b8 - 0x5b0];
    // Below 1 when the game runs slow.
    float gameSpeed;
    MidiPlayer *midi;
    unsigned char pad_5c0[0x5cc - 0x5c0];
    // The ANM file of the menus' text, as th08's.
    AnmLoaded *textAnm;
    // The ANM file of the loading sprites.
    AnmLoaded *loadingAnm;
    union
    {
        DWORD stateFlags;
        struct
        {
            unsigned int unusedBits5d4_0 : 9;
            // Whether the scene's objects load their resources, as th08's.
            unsigned int loadsResources : 1;
        };
    };
    // The device's capabilities, as th08's.
    D3DCAPS8 caps;
    // The loading thread, as th08's.
    HANDLE subThread;
    DWORD subThreadId;
    // Set to ask the sub thread to end.
    int closeSubThread;
    int unknown6b8;
    // Set while LoadingThread runs, as th08's.
    int loadingThreadState;
    CRITICAL_SECTION criticalSections[5];
    // How deep each of criticalSections is entered.
    BYTE criticalSectionDepths[5];
    unsigned char pad_73d[0x740 - 0x73d];
    // 0 while the loading sprites are hidden, 1 while they show, 2 once
    // they are finishing.
    int loadingState;
    unsigned char pad_744[0x77c - 0x744];
    int unknown77c;
    unsigned char pad_780[0x784 - 0x780];
    int unknown784;
    int unknown788;
    int unknown78c;
    int unknown790;
    // Whether fog is on, as EnableFog and DisableFog last set it.
    DWORD fogState;
    // timeGetTime() when AddPlayTime and AddTotalPlayTime last ran.
    DWORD lastPlayTimeTick;
    DWORD lastTotalPlayTimeTick;
    // The executable's checksum and size, for CheckVersionFile, as
    // th08's.
    DWORD exeChecksum;
    DWORD exeSize;
    // The contents of the version file, as th08's: lines of a version,
    // a size and a checksum.
    DWORD verFileSize;
    char *verFile;
};
extern App g_App;

// Whether the game's objects free their resources (0x415cf0).
int FreesResources();

// The loading sprites, at 0x4b38b0.
extern Sprite g_LoadingSprites[3];

// DirectInput enumeration callbacks: creates the first two pads, as
// th08's EnumJoysticksCallback (0x42edd0), and sets the range of the
// first's and the second's axes to -1000..1000 (0x42ee30, 0x42ee90).
BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE *pdidInstance, VOID *pContext);
BOOL CALLBACK EnumAxesCallback(const DIDEVICEOBJECTINSTANCE *pdidoi, VOID *pContext);
BOOL CALLBACK EnumAxesCallback2(const DIDEVICEOBJECTINSTANCE *pdidoi, VOID *pContext);

// Loads score.dat (0x421ea0), and writes it back through scoretmp
// (0x421f10).
int LoadScore();
int SaveScore();

class ScreenEffect;
// The loading screen's fade, at 0x4b30f8.
extern ScreenEffect *g_LoadingScreenEffect;
