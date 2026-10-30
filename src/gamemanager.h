// The GameManager: the state of a game, played by two sides.

#pragma once

#include "chain.h"
#include "unsorted.h"

// A side's statistics.
struct GameStats
{
    // th08's: AddExtend gives one up to 7.
    float lives;
    // The score shown, and the score, counting in tens, as th08's.
    DWORD guiScore;
    DWORD score;
    unsigned char pad_c[0x4];
    // The extends given; 99999 once there are no more.
    int extendsGiven;
    BYTE unknown14;
};

// One side of the game (the third is used for little).
struct GameSide
{
    void *unknown00;
    void *unknown04;
    void *unknown08;
    void *unknown0c;
    void *unknown10;
    void *unknown14;
    void *unknown18;
    GameStats *stats;
    int unknown20;
    int unknown24;
    unsigned char pad_28[0x34 - 0x28];
    DWORD flags;
};
ASSERT_SIZE(GameSide, 0x38);

// th08's GameManager, at 0x4a7d90. Only the fields known so far.
class GameManager
{
public:
    // Zeroes the GameManager, sets the play area, and sets unknown378 and
    // unknown37c to 10, as th08's constructor did (0x41ba4f).
    void Init();

    // The draw chain callback, as th08's: moves isPaused on from 1 to 2.
    // Returns 1, or 3 outside scene 2 or in the result screen's mode
    // (bits 5-6 of stateFlags) 1 (0x41a75f).
    static int OnDraw(GameManager *mgr);
    // The update chain callback and its added and deleted callbacks
    // (0x41aa5f, 0x41b76f, 0x41b82a).
    static int OnUpdate(GameManager *mgr);
    static int AddedCallback(GameManager *mgr);
    static int DeletedCallback(GameManager *mgr);
    // Sets up and adds the update and draw chain elements, with
    // priorities 2 and 5, as th08's RegisterGameManagerChain. Returns 0,
    // or -1 if the update element's added callback failed (0x41b9d0).
    static int RegisterChain();
    // Whether a `width` by `height` box centred on (`x`, `y`) touches the
    // play area, as th08's; th09's is centred on x = 0 (0x41a6eb).
    BOOL IsInPlayArea(float x, float y, float width, float height);
    // Clears bit 0 of every side's flags and resets the state of a game
    // (0x41a7c3).
    void ResetState();
    // Whether a demo plays: bit 1 of stateFlags (0x41a9ac).
    int IsDemo();
    // Sets arena to the play area (-144, 0, 288, 448) and arena2 to
    // (-136, 16, 272, 416), as th08's. Inline, but not expanded in Init,
    // which calls the copy at 0x41a9b8.
    void SetPlayArea()
    {
        this->arena[0] = -144.0f;
        this->arena[1] = 0.0f;
        this->arena[2] = 288.0f;
        this->arena[3] = 448.0f;
        this->arena2[0] = -136.0f;
        this->arena2[1] = 16.0f;
        this->arena2[2] = 272.0f;
        this->arena2[3] = 416.0f;
    }
    // Gives side 0 a life, up to 7, with sound 0x1c, and counts the
    // extend; past the last (8 extends in mode 1, else 5) there are no
    // more (0x41aa07).
    void AddExtend();

    GameSide sides[3];
    void *unknownA8;
    void *unknownAc;
    int unknownB0;
    unsigned char pad_b4[0x4];
    int unknownB8;
    int unknownBc;
    int unknownC0;
    unsigned char pad_c4[0xd0 - 0xc4];
    int unknownD0;
    void *unknownD4;
    unsigned char pad_d8[0x110 - 0xd8];
    int unknown110;
    unsigned char pad_114[0x4];
    int mode;
    unsigned char pad_11c[0x124 - 0x11c];
    int unknown124;
    int unknown128;
    int unknown12c;
    int unknown130;
    DWORD stateFlags;
    unsigned char pad_138[0x13c - 0x138];
    // 1 once paused, 2 once the pause was drawn.
    BYTE isPaused;
    BYTE unknown13d;
    BYTE unknown13e;
    BYTE unknown13f;
    unsigned char pad_140[0x348 - 0x140];
    int frames;
    unsigned char pad_34c[0x350 - 0x34c];
    float arena[4];
    float arena2[4];
    unsigned char pad_370[0x378 - 0x370];
    int unknown378;
    int unknown37c;
    int unknown380;
    unsigned char pad_384[0x390 - 0x384];
};
ASSERT_SIZE(GameManager, 0x390);
extern GameManager g_GameManager;
// Its update and draw chain elements (0x4a8120, 0x4a8140).
extern ChainElem g_GameManagerCalcChain;
extern ChainElem g_GameManagerDrawChain;

// Removes the GameManager's chain elements, caps both sides' score at
// 999999999 and copies it to guiScore, and resets gameSpeed, as th08's
// (0x41a827).
void RemoveGameManagerChain();

// Writes today's date to `buf` as "yy/mm/dd" (0x41a796).
void FormatDate(char *buf);
