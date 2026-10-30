// The player, one per side ("PlayerInf").

#pragma once

#include "chain.h"
#include "gamemanager.h"
#include "sprite.h"
#include "timer.h"

// One of the player's hit areas (0x44 bytes), th08's PlayerBombHitbox
// grown. Placeholder name; only the fields known so far.
struct PlayerHitArea
{
    // Zeroes it, then sets unknown34 to 1, as th08's Reset (0x41bda0).
    void Reset();
    // Clears active, as th08's (0x41bd90).
    void Deactivate();

    float x;
    float y;
    // A circle's radius, and what it grows by, as th08's.
    float radius;
    float radiusGrowth;
    // A box's size, as th08's.
    float width;
    float height;
    unsigned char pad_18[0x24 - 0x18];
    // Counts down, as th08's.
    int framesLeft;
    int unknown28;
    unsigned char pad_2c[0x34 - 0x2c];
    int unknown34;
    // Its shape or kind: 0 and 1 boxes, 2 and 4 circles or boxes as the
    // adders set it.
    int kind;
    BYTE active;
    unsigned char pad_3d[0x40 - 0x3d];
    int unknown40;
};
ASSERT_SIZE(PlayerHitArea, 0x44);

// A part of the player at +0x24 with three timers (0x78 bytes).
// Placeholder name.
struct PlayerUnknown24
{
    // Sets timer5c and timer68 to 0 and clears the low two flags
    // (0x41bdc0).
    void Reset();

    Timer timer0;
    unsigned char pad_c[0x5c - 0xc];
    Timer timer5c;
    Timer timer68;
    DWORD flags;
};
ASSERT_SIZE(PlayerUnknown24, 0x78);

// An entry of PlayerUnknown36c (0x30 bytes). Its implicit constructor
// folds with D3DXVECTOR3's. Placeholder name.
struct PlayerUnknown36cEntry
{
    D3DXVECTOR3 pos;
    unsigned char pad_c[0x30 - 0xc];
};
ASSERT_SIZE(PlayerUnknown36cEntry, 0x30);

// A part of the player at +0x36c: the player and 0x80 entries.
// Placeholder name.
struct PlayerUnknown36c
{
    Player *player;
    PlayerUnknown36cEntry entries[0x80];
};
ASSERT_SIZE(PlayerUnknown36c, 0x1804);

// One of the player's options (0x2f4 bytes), the size and timer of
// th08's PlayerOption. Only the fields known so far.
struct PlayerOption
{
    Sprite sprite;
    unsigned char pad_2a4[0x2e0 - 0x2a4];
    Timer timer;
    unsigned char pad_2ec[0x2f4 - 0x2ec];
};
ASSERT_SIZE(PlayerOption, 0x2f4);

// One of the player's bullets (0x484 bytes), the layout of th08's
// PlayerBullet. Only the fields known so far.
struct PlayerBullet
{
    Sprite sprite;
    D3DXVECTOR3 pos;
    D3DXVECTOR3 trail[0x20];
    unsigned char pad_430[0x450 - 0x430];
    // The direction it flies in.
    float angle;
    Timer timer;
    short damage;
    // 0 unused, 1 flying, 2 hit, as th08's.
    short state;
    unsigned char pad_464[0x46e - 0x464];
    // Drawn blue while it is hit.
    BYTE unknown46e;
    unsigned char pad_46f[0x484 - 0x46f];
};
ASSERT_SIZE(PlayerBullet, 0x484);

// A part of the player at +0x3031c with a timer (0x1c bytes). Its
// constructor folds with BulletEx's. Placeholder name.
struct PlayerUnknown3031c
{
    Timer timer;
    unsigned char pad_c[0x1c - 0xc];
};
ASSERT_SIZE(PlayerUnknown3031c, 0x1c);

// A part of the player at +0x30410, which points back to it (0x40
// bytes). Placeholder name.
struct PlayerUnknown30410
{
    // Counts a hit and scores it; `pos` is where (0x41d150).
    void AddCombo(D3DXVECTOR3 *pos, int a, int b, int c, int d);
    // AddCombo, counting unknown38 too (0x40f8b0).
    void AddComboCounted(D3DXVECTOR3 *pos, int a, int b, int c, int d);
    // Sets timer20, count and unknown38 to 0 (0x40f8c0).
    void ResetCombo();
    // Runs the two timers down: when timer20 runs out the count is
    // reset, and when timer14 does the combo ends as in EndCombo
    // (0x41d760).
    void Update();
    // Resets the count, adds bonus to the side's score and clears it
    // (0x41d7e0).
    void EndCombo();

    Player *player;
    // The hits counted, and the most of them.
    int count;
    int maxCount;
    int unknownC;
    // Added to the side's score when the combo ends.
    int bonus;
    Timer timer14;
    Timer timer20;
    unsigned char pad_2c[0x38 - 0x2c];
    int unknown38;
    int unknown3c;
};
ASSERT_SIZE(PlayerUnknown30410, 0x40);

// One of four sprites of the player at +0x30454 (0x2c4 bytes).
// Placeholder name.
struct PlayerUnknown30454
{
    unsigned char pad_0[0x20];
    Sprite sprite;
};
ASSERT_SIZE(PlayerUnknown30454, 0x2c4);

// A side's player (0x30f70 bytes). Only the fields known so far.
class Player
{
public:
    // Empty, and defined out of line, as th08's: constructs the members
    // (0x41f070). Their implicit constructors come with it (0x41ee00,
    // 0x41ceb0, 0x41f010, 0x41f030, 0x41ee20, 0x41ee30, 0x41f060).
    Player();

    // Allocates and zeroes a player for side `sideIdx`, keeping `arg` in
    // registerChainArg and `unknown20`, and adds it to the chains (update
    // priority sideIdx + 0xf, draw sideIdx + 0xc and sideIdx + 0xe).
    // Returns it, even if adding fails (0x41f170).
    static Player *RegisterChain(BYTE arg, int sideIdx, int unknown20);
    // The chain callbacks (0x41e900, 0x41eb10, 0x41cdc0, 0x41ee50).
    static int OnUpdate(Player *player);
    static int OnDraw(Player *player);
    static int OnDraw2(Player *player);
    static int AddedCallback(Player *player);
    // Frees the player's resources, removes its chain elements and
    // deletes it, if any (0x41ced0).
    static void RemoveChain(Player *player);
    // Resets the player for a new round (0x41ebc0).
    void Reset();
    // Sets registerChainArg (0x41bde0).
    void SetRegisterChainArg(BYTE arg);
    // Takes a free hit area (the last of the free list), or the spare
    // when one is left, resets it and starts it at `pos` with
    // `framesLeft` and `unknown40` (0x41cf30).
    PlayerHitArea *AllocHitArea(D3DXVECTOR3 *pos, int framesLeft, int unknown40);
    // Lowers unknown30388 by `amount`, down to 1 (0x41f310).
    void SubUnknown30388(float amount);
    // Draws the bullets that hit (state 2) at their positions on the
    // side's screen, turned to their angle when they rotate, blue while
    // unknown46e is set, as th08's DrawHitBullets (0x41f760).
    void DrawHitBullets();

    unsigned char pad_0[0x4];
    // RegisterChain's first argument.
    BYTE registerChainArg;
    unsigned char pad_5[0x8 - 0x5];
    // The side, 0 or 1.
    int sideIdx;
    GameSide *side;
    GameSide *otherSide;
    ChainElem *calcElem;
    ChainElem *drawElem;
    ChainElem *drawElem2;
    int unknown20;
    PlayerUnknown24 unknown24;
    unsigned char pad_9c[0xbc - 0x9c];
    // The character's ANM, in slot sideIdx + 5.
    AnmLoaded *anm;
    Sprite sprite;
    unsigned char pad_364[0x36c - 0x364];
    PlayerUnknown36c unknown36c;
    unsigned char pad_1b70[0x1b74 - 0x1b70];
    Timer unknown1b74;
    unsigned char pad_1b80[0x1b88 - 0x1b80];
    D3DXVECTOR3 pos;
    unsigned char pad_1b94[0x1ba0 - 0x1b94];
    // The player's last positions, as th08's.
    D3DXVECTOR3 posHistory[0x10];
    unsigned char pad_1c60[0x1cec - 0x1c60];
    PlayerOption options[4];
    // The hit areas; the last is the spare AllocHitArea hands out when
    // none is free.
    PlayerHitArea hitAreas[0x201];
    // The hit areas in use and the free ones, and how many of each.
    PlayerHitArea *activeHitAreas[0x202];
    PlayerHitArea *freeHitAreas[0x202];
    int activeHitAreaCount;
    int freeHitAreaCount;
    unsigned char pad_c118[0xc11c - 0xc118];
    PlayerBullet bullets[0x80];
    PlayerUnknown3031c unknown3031c;
    // The character's shot file (plXX.sht).
    void *shtFile;
    unsigned char pad_3033c[0x30388 - 0x3033c];
    // Lowered by SubUnknown30388, down to 1.
    float unknown30388;
    Timer unknown3038c;
    Timer unknown30398;
    Timer unknown303a4;
    Timer unknown303b0;
    Timer unknown303bc;
    Timer unknown303c8;
    Timer unknown303d4;
    unsigned char pad_303e0[0x30410 - 0x303e0];
    PlayerUnknown30410 unknown30410;
    unsigned char pad_30450[0x30454 - 0x30450];
    PlayerUnknown30454 unknown30454[4];
    unsigned char pad_30f64[0x30f70 - 0x30f64];
};
ASSERT_SIZE(Player, 0x30f70);

// Each side's shot file, kept while the side keeps its character
// (0x4a8160).
extern void *g_ShtFiles[2];
