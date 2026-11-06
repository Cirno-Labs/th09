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
// where code calls them: cosf (0x401060), sinf (0x401070) and sqrtf
// (0x401080), a single x87 instruction each under /Oi. Until their
// callers are decompiled, the rest are named wrappers.
// More of them: atan2f (0x405700), fmodf through _CIfmod (0x405710),
// fabsf (0x40f1c0), tanf (0x42e900), acosf through _CIacos (0x436aa0)
// and atanf (0x436ab0).
inline float Atan2(float y, float x)
{
    return (float)atan2((double)y, (double)x);
}
float Fmod(float x, float y);
inline float Fabs(float x)
{
    return (float)fabs((double)x);
}
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
    WORD GetRandomU16InRange(WORD range)
    {
        return range != 0 ? this->GetRandomU16() % range : 0;
    }
    // range != 0 ? GetRandomU32() % range : 0, as th08's inline
    // function; this copy is 0x4048e0.
    DWORD GetRandomU32InRange(DWORD range)
    {
        return range != 0 ? this->GetRandomU32() % range : 0;
    }
    // A random float from 0 up to `range` (0x404910), and from -`range`
    // up to `range` (0x406200).
    float GetRandomF32InRange(float range)
    {
        return this->GetRandomF32ZeroToOne() * range;
    }
    float GetRandomF32SignedInRange(float range)
    {
        return this->GetRandomF32MinusOneToOne() * range;
    }

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
    // Its registered exit callback is an empty return (0x48da70).
    ~Log() {}

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
extern Log g_Logger;

// The game's window, as th08's (0x44 bytes, at 0x4b30b0). Only the
// fields known so far.
#pragma pack(push, 4)
struct GameWindow
{
    GameWindow();
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
// joysticks' capabilities into g_JoyDeviceCaps and returns 0, as th08's for
// one (0x42c8f0).
WORD InitJoystick();
// The joysticks' capabilities, from joyGetDevCaps (0x4ac8d8).
extern JOYCAPSA g_JoyDeviceCaps[2];

// Adds what pad `pad` (0 or 1) has down to `buttons` by
// g_PlayerInputs[input]'s first mapping, and the stick as the
// directions, as th08's ReadJoystick for one pad; joyGetPosEx reads it
// unless both DirectInput pads are there (0x42b410).
WORD ReadJoystick(int input, int pad, WORD buttons);
// Player `player`'s pad buttons, 0x80 for each one down, as th08's
// GetPadButtons: through DirectInput, or joyGetPosEx when the player
// has no DirectInput pad (0x42b740). Returns g_PadButtons.
BYTE *GetPadButtons(int player);
// The buttons GetPadButtons read (0x4b3028).
extern BYTE g_PadButtons[0x80];

// Puts `buttons` in g_PlayerInputs[player]: the last ones go to
// unk_2, those just pressed to unk_6 and just released to
// unk_8, and unk_4 gets the held ones' repeats, every 8 frames from
// the 26th. Returns the input buttons (0x42ace0).
WORD UpdatePlayerInput(int player, WORD buttons);

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
// g_PlayerInputs[player] and returns them (0x42b850). The keyboard layout
// is the player's pad setting: 2 (always for player 2) the arrows and
// numpad with Z, X and shift, 3 TFHB with RYVN as the diagonals, 4 the
// numpad with down, right and left as shot, bomb and focus. Player 2
// also reads both pads.
WORD ReadButtons(int player);
// Reads the local keyboard and the pads of both players' settings
// into one set of buttons, for network play (0x42be40).
WORD ReadNetInput();
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
    BYTE pad_3;
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
void *LoadFileData(char *path, int *size, int isExternal);

// a + b, brought back into -pi..pi in at most 17 steps, as th08's
// (0x42aed0).
float AddNormalizeAngle(float a, float b);
// The sine and cosine of `angle`, with one fsincos (0x412780).
inline void SinCos(float *sinOut, float *cosOut, float angle)
{
    __asm
    {
        fld angle
        fsincos
        mov eax, cosOut
        fstp [eax]
        mov eax, sinOut
        fstp [eax]
    }
}
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
    // The same with radiusX for x and radiusY for y (0x40d4d0).
    void SetPolarEllipse(float angle, float radiusX, float radiusY);

    float x;
    float y;
};

// Sets v's x and y to `radius` along `angle`, with one fsincos. Static in
// a header: each file compiles its own copy before its callers, which do
// not reload ecx after it, and the linker folds them into one at
// 0x441890.
static void SetPolar(Vector2 *v, float angle, float radius)
{
    __asm
    {
        mov eax, v
        fld angle
        fsincos
        fmul radius
        fstp [eax]
        fmul radius
        fstp [eax+4]
    }
}

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
    // Default joystick, virtual-key and DirectInput mappings (0x42b010).
    PlayerInput();

    // Whether a button of `mask` is down in either of the masks at 4
    // and 6 (0x423158).
    WORD IsPressed(WORD mask)
    {
        return (WORD)(this->unk_6 & mask) != 0 || (WORD)(this->unk_4 & mask) != 0;
    }

    // unk_0 & mask and unk_6 & mask (0x40e2b0, 0x40e2d0), and the
    // same for the second set, unk_2c and unk_32 (0x415cb0,
    // 0x415cd0).
    WORD IsHeld(WORD mask)
    {
        return this->unk_0 & mask;
    }
    WORD IsJustPressed(WORD mask)
    {
        return this->unk_6 & mask;
    }
    WORD IsHeld2(WORD mask);
    WORD IsJustPressed2(WORD mask);
    // Clears unk_4 and updates the second set from unk_2c and
    // unk_2e: counts how long each button is held, setting bit 0 of
    // unk_30 every 8 frames from the 26th, and takes the buttons just
    // pressed into unk_32 and just released into unk_34
    // (0x4200f0).
    void UpdateRepeat2();

    WORD unk_0;
    WORD unk_2;
    WORD unk_4;
    WORD unk_6;
    WORD unk_8;
    // How long each button of unk_0 has been held.
    WORD heldFrames[16];
    // How long button 1 of the second set has been held during a
    // replay, up to 8, for the sides whose config unk_b4 is 1.
    WORD unk_2a;
    WORD unk_2c;
    WORD unk_2e;
    WORD unk_30;
    WORD unk_32;
    WORD unk_34;
    unsigned char pad_36[0x38 - 0x36];
    // How long each button of unk_2c has been held.
    WORD heldFrames2[16];
    // The default mappings.
    ControllerMapping mappings[3];
};
ASSERT_SIZE(PlayerInput, 0x8e);
extern PlayerInput g_PlayerInputs[3];

// The header of thbgm.dat, as th08's.
struct BgmDatHeader
{
    // 'ZWAV'.
    DWORD magic;
    int version;
    int gameVersion;
    DWORD unk_c;
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
    // Nonzero in mode 0 has Gui::ScoreRoundBonus show sprite 0x41 and
    // give no bonus.
    BYTE unk_ac;
    // Whether the full screen is 16-bit; 0xff until the first start,
    // as th08's.
    BYTE colorMode16Bits;
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
    BYTE unk_b3;
    // By player; the key config switches each between 0 and 1.
    BYTE unk_b4[3];
    // The pad each player's keys are read from: 0 and 1 are the
    // joysticks (GetPadButtons), 2 to 4 keyboard layouts; the two
    // players' cannot clash.
    BYTE pads[3];
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

// A record in App's lists.
struct RunRecord
{
    WORD unk_0;
    WORD seed;
    int value;
    int unk_8;
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
    // `at`, normalized, as th08's Stage camera's.
    D3DXVECTOR3 dir;
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

#include "app.h"
