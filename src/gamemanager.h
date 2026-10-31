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
    // Goes to Hscr::unknown2b.
    BYTE unknown14;
};

class Stage;
class Player;
class BulletManager;
class EffectManager;
class EnemyManager;
class SpellCard;
class GuiSide;
class Gui;
class ExAttackManager;
class ReplayManager;

// One side of the game (the third is used for little).
struct GameSide
{
    // Adds a tenth of `points` to stats->score, which counts in tens
    // (0x40f860).
    void AddScore(int points);

    // The side's objects, which GameManager::DeletedCallback removes.
    Stage *stage;
    Player *player;
    BulletManager *bulletManager;
    EffectManager *effectManager;
    EnemyManager *enemyManager;
    SpellCard *spellCard;
    GuiSide *gui;
    GameStats *stats;
    // The character: its files are listed by it, and Player keeps the
    // shot file while unknown24, the last game's, is the same.
    int character;
    int unknown24;
    int unknown28;
    unsigned char pad_2c[0x30 - 0x2c];
    // Picks the character's second ANM (plXXb.anm) when set.
    int unknown30;
    union
    {
        DWORD flags;
        struct
        {
            // Set by the enemy script function SetSidesFlag0 on both
            // sides.
            DWORD flag0 : 1;
        };
    };
};
ASSERT_SIZE(GameSide, 0x38);

// GameManager's fields at 0xb0-0xd3, kept under the offsets in
// GameManager they had before.
// Something of shorts GameManager points to; in mode 2 the dialogue
// plays music unknownC (the copy at 0x416dce, unknown6, 8 and A in the
// front).
struct GameManagerUnknownF0
{
    unsigned char pad_0[0x6];
    short unknown6;
    short unknown8;
    short unknownA;
    short unknownC;
};

struct GameRank
{
    // Lowers unknownCc by `amount`, to at least 0. Inline; the copy is
    // at 0x403e30.
    void LowerUnknownCc(int amount)
    {
        this->unknownCc -= amount;
        if (this->unknownCc < 0)
        {
            this->unknownCc = 0;
        }
    }

    int unknownB0;
    // ResetRound lowers it by 2 per unknown100, to at least 1, from a
    // value set by difficulty; in mode 2 by 8, to at least unknown100.
    int unknownB4;
    int unknownB8;
    int unknownBc;
    int unknownC0;
    // Set by ResetRound from difficulty, unknown100 and unknownFc.
    int unknownC4;
    int unknownC8;
    // ResetRound lowers it by 1000, to at least 0.
    int unknownCc;
    int unknownD0;
};
ASSERT_SIZE(GameRank, 0x24);

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
    // (0x41aa5f, 0x41b76f, 0x41b82a). The added callback, as th08's,
    // sets App::loadsResources by the scene, sets loadingThreadState,
    // starts the loading sprites (and, coming from the title, the loading
    // screen effect), turns the result screen's mode 2 and 3 into 1 and
    // runs LoadingThread in the sub thread. Returns 0.
    static int OnUpdate(GameManager *mgr);
    static int AddedCallback(GameManager *mgr);
    static int DeletedCallback(GameManager *mgr);
    // The rest of the added callback, run in the sub thread (0x41af2d).
    static void LoadingThread(GameManager *arg);
    // Sets up and adds the update and draw chain elements, with
    // priorities 2 and 5, as th08's RegisterGameManagerChain. Returns 0,
    // or -1 if the update element's added callback failed (0x41b9d0).
    static int RegisterChain();
    // The screen position of `x` or `y` in the play area of the view
    // App::SetView set last. Inline; the copies are the stage's object's
    // (0x401680, 0x4016b0).
    float ArenaToScreenX(float x)
    {
        return g_App.camera->viewport.X + x - this->arena[0];
    }
    float ArenaToScreenY(float y)
    {
        return g_App.camera->viewport.Y + y - this->arena[1];
    }
    // The reverse: the play area position of screen position `x` or `y`.
    // Inline; the copies are pl02.cpp's (0x442420, 0x442450).
    float ScreenToArenaX(float x)
    {
        return x - g_App.camera->viewport.X + this->arena[0];
    }
    float ScreenToArenaY(float y)
    {
        return y - g_App.camera->viewport.Y + this->arena[1];
    }
    // Whether a `width` by `height` box centred on (`x`, `y`) touches the
    // play area, as th08's; th09's is centred on x = 0 (0x41a6eb).
    BOOL IsInPlayArea(float x, float y, float width, float height);
    // Clears bit 0 of every side's flags and resets the state of a game
    // (0x41a7c3).
    void ResetState();
    // Whether a demo plays: bit 1 of stateFlags (0x41a9ac).
    int IsDemo();
    // Whether a replay plays: bit 3 of stateFlags, as th08's. Inline;
    // the copy the others call is at 0x415d20.
    int IsReplay()
    {
        return this->stateFlags >> 3 & 1;
    }
    // Whether mode is 2 (0x415d30).
    BOOL IsMode2();
    // Resets the state, the ASCII text and every object for a new round,
    // and sets unknownB4, unknownC4 and unknownC8 by difficulty
    // (0x41b5c6).
    static void ResetRound();
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
    // Whether mode is 0 (0x40e470).
    BOOL IsMode0();
    // Whether mode is 1 (0x404970).
    BOOL IsMode1();

    GameSide sides[3];
    Gui *gui;
    ExAttackManager *exAttackManager;
    GameRank rank;
    ReplayManager *replayManager;
    unsigned char pad_d8[0xe8 - 0xd8];
    // Deleted by DeletedCallback.
    void *unknownE8;
    // How far the enemy timelines advance a frame.
    float timelineSpeed;
    GameManagerUnknownF0 *unknownF0;
    // The stage, an index into g_StageFiles. DeletedCallback copies it
    // to unknownF8, as each side's character to unknown24.
    int stage;
    int unknownF8;
    int unknownFc;
    int unknown100;
    // The rounds a side must win, and each side's wins.
    int unknown104;
    int unknown108[2];
    int unknown110;
    int unknown114;
    int mode;
    int difficulty;
    // The difficulty's bit, as th08's: timeline entries run on the
    // difficulties in their bits.
    BYTE difficultyMask;
    unsigned char pad_121[0x124 - 0x121];
    // Set while LoadingThread runs, as th08's.
    int loadingThreadState;
    int unknown128;
    int unknown12c;
    int unknown130;
    union
    {
        DWORD stateFlags;
        struct
        {
            unsigned int unusedBits134_0 : 5;
            // The result screen's mode, as th08's: OnDraw returns 3 in
            // mode 1, and AddedCallback turns 2 and 3 into 1.
            unsigned int stageEndState : 2;
            unsigned int unusedBits134_7 : 4;
            // A bit per side, cleared by SpellCard::OnUpdate 30 frames
            // into the side's spell card.
            unsigned int unknownBits134_11 : 2;
        };
    };
    unsigned char pad_138[0x13c - 0x138];
    // 1 once paused, 2 once the pause was drawn.
    BYTE isPaused;
    // Whether the continue menu is up, as th08's.
    BYTE isInContinueMenu;
    // Whether AsciiText's unknownMenu is up.
    BYTE unknown13e;
    // The demo the title plays next.
    BYTE demoIndex;
    unsigned char pad_140[0x145 - 0x140];
    // The replay to play.
    char replayFile[0x200];
    unsigned char pad_345[0x348 - 0x345];
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
// A high score entry ("HSCR", version 2 now), as th08's. Only the fields
// known so far.
struct Hscr
{
    DWORD magic;
    WORD size;
    // The size again.
    WORD size2;
    BYTE version;
    // Set on the entry being named.
    BYTE isNew;
    unsigned char pad_a[0x2];
    DWORD score;
    // How much the game slowed down, in percent.
    float slowdown;
    BYTE character;
    BYTE difficulty;
    // The entry's place in its table, from 0.
    BYTE rank;
    BYTE unknown17;
    char name[9];
    // "--/--" when unset.
    char date[6];
    unsigned char pad_27[0x2b - 0x27];
    BYTE unknown2b;
};
ASSERT_SIZE(Hscr, 0x2c);
// The entry of the game that ended, set up by GameManager's deleted
// callback (0x4ac858).
extern Hscr g_Hscr;
// The high score tables: five entries for each character and
// difficulty (0x4a8380).
extern Hscr g_HighScores[16][5][5];
// A block GameManager's deleted callback clears (0x4d9698).
extern unsigned char g_Unknown4d9698[0x2bc0];

// Its update and draw chain elements (0x4a8120, 0x4a8140).
extern ChainElem g_GameManagerCalcChain;
extern ChainElem g_GameManagerDrawChain;

// Removes the GameManager's chain elements, caps both sides' score at
// 999999999 and copies it to guiScore, and resets gameSpeed, as th08's
// (0x41a827).
void RemoveGameManagerChain();

// Writes today's date to `buf` as "yy/mm/dd" (0x41a796).
void FormatDate(char *buf);
