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
    // How far guiScore counts up a frame, as th08's.
    DWORD scoreIncrement;
    // The extends given; 99999 once there are no more.
    int extendsGiven;
    // Goes to Hscr::unknown2b.
    BYTE unknown14;
    unsigned char pad_15[0x34 - 0x15];
    // Gets the life the losing side 1 gives up at the end of a round.
    float unknown34;
    unsigned char pad_38[0xa0 - 0x38];
};
ASSERT_SIZE(GameStats, 0xa0);

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
    void AddScore(int points)
    {
        this->stats->score += points / 10;
    }

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
    int unknown2c;
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

// An opponent of story mode, or a match's stage: lists of them end with
// a negative round.
struct GameManagerUnknownF0
{
    // The story round (GameManager::unknownFc) it can be picked in; 9 in
    // the match list.
    short round;
    short stage;
    // Side 1's character, or the character whose stage it is.
    short character;
    short unknown6;
    short unknown8;
    short unknownA;
    short unknownC;
    // How likely PickOpponent picks it among the round's.
    short weight;
};
ASSERT_SIZE(GameManagerUnknownF0, 0x10);

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

    // Frames played; OnUpdate counts it while no dialogue runs.
    DWORD unknownB0;
    // ResetRound lowers it by 2 per unknown100, to at least 1, from a
    // value set by difficulty; in mode 2 by 8, to at least unknown100.
    int unknownB4;
    int unknownB8;
    int unknownBc;
    DWORD unknownC0;
    // Set by ResetRound from difficulty, unknown100 and unknownFc.
    DWORD unknownC4;
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
    GameManager()
    {
        this->Init();
    }

    // Zeroes the GameManager, sets the play area, and sets initialLives
    // to 10, as th08's constructor did (0x41ba4f).
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
    // runs LoadingThread in the sub thread. Returns 0. The update
    // callback copies the menu input to both sides, handles pausing and
    // the demo, advances the rank, spawns a pair of item enemies once
    // rank.unknownCc passes 10000, rolls the shown scores up and gives
    // the extends. Returns 1, or 3 while paused or loading.
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
    // Whether a practice game plays: bit 0 of stateFlags, as th08's.
    // Inline; the copy the pause menu calls is at 0x4343b0.
    int IsInPractice()
    {
        return this->stateFlags & 1;
    }
    // Whether mode is 2 (0x415d30).
    BOOL IsMode2();
    // Resets the state, the ASCII text and every object for a new round,
    // and sets unknownB4, unknownC4 and unknownC8 by difficulty
    // (0x41b5c6).
    static void ResetRound();
    // Sets arena to the play area (-144, 0, 288, 448) and playerArea to
    // (-136, 16, 272, 416), as th08's. Inline, but not expanded in Init,
    // which calls the copy at 0x41a9b8.
    void SetPlayArea()
    {
        this->arena[0] = -144.0f;
        this->arena[1] = 0.0f;
        this->arena[2] = 288.0f;
        this->arena[3] = 448.0f;
        this->playerArea[0] = -136.0f;
        this->playerArea[1] = 16.0f;
        this->playerArea[2] = 272.0f;
        this->playerArea[3] = 416.0f;
    }
    // Gives side 0 a life, up to 7, with sound 0x1c, and counts the
    // extend; past the last (8 extends in mode 1, else 5) there are no
    // more (0x41aa07).
    void AddExtend();
    // Whether mode is 0 (0x40e470).
    BOOL IsMode0()
    {
        return this->mode == 0;
    }
    // Whether mode is 1 (0x404970).
    BOOL IsMode1()
    {
        return this->mode == 1;
    }
    // Picks the stage and side 1's character: in modes 0 and 1 a random
    // opponent of the round from side 0's character's list, by weight,
    // one not met yet (unknownD8), also setting side 1's unknown2c; else
    // the match stage of unknown138's character (-1 either side's, -2 a
    // random open one) (0x415910).
    void PickOpponent();

    GameSide sides[3];
    Gui *gui;
    ExAttackManager *exAttackManager;
    GameRank rank;
    ReplayManager *replayManager;
    // By character; a replay of mode 0 or 1 sets those of side 1's
    // earlier rounds.
    BYTE unknownD8[0x10];
    // Deleted by DeletedCallback. A copy of the config, made by
    // LoadingThread (a replay restores its own into it).
    GameConfig *config;
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
    int difficultyMask;
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
            // Set by AsciiText's continue (13; 14 with it in some modes),
            // cleared by LoadingThread.
            unsigned int unknownBit134_13 : 1;
            unsigned int unknownBit134_14 : 1;
        };
    };
    int unknown138;
    // 1 once paused, 2 once the pause was drawn.
    BYTE isPaused;
    // Whether the continue menu is up, as th08's.
    BYTE isInContinueMenu;
    // Whether AsciiText's unknownMenu is up.
    BYTE unknown13e;
    // The demo the title plays next.
    BYTE demoIndex;
    // How long the demo has played.
    int demoFrames;
    unsigned char pad_144[0x145 - 0x144];
    // The replay to play.
    char replayFile[0x200];
    unsigned char pad_345[0x346 - 0x345];
    // The seed the game started with, which the replay records.
    WORD rngSeed;
    int frames;
    unsigned char pad_34c[0x350 - 0x34c];
    float arena[4];
    float playerArea[4];
    unsigned char pad_370[0x374 - 0x370];
    // The item of the last pair of enemies OnUpdate spawned.
    int unknown374;
    // Each side's lives at the start of a round, in halves; Player::Reset
    // sets the player's unknownA8 from it.
    int initialLives[2];
    int unknown380;
    unsigned char pad_384[0x38c - 0x384];
    // Set by the loading thread, as th08's pendingMusicStart: OnUpdate
    // releases surface 8 and, at 1, plays music 0.
    int unknown38c;
};
ASSERT_SIZE(GameManager, 0x390);
extern GameManager g_GameManager;
// A high score entry ("HSCR", version 2 now), as th08's. Only the fields
// known so far.
struct Hscr
{
    Hscr();

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
// Side 1's unknown2c at the start of a round of mode 0 or 1, by
// difficulty and GameManager::unknownFc: [0] when side 0 has lives and
// unknown14 is clear, else [1 + unknown100] up to [4] (0x4a1500).
extern int g_Unknown4a1500[5][9][5];
// Both sides' unknown2c in mode 2, by difficulty (0x4a1884).
extern int g_Unknown4a1884[5];
// Each story character's opponents (0x4a14c8), and the stages of the
// matches by character (0x4a13b8).
extern GameManagerUnknownF0 *g_StoryOpponents[14];
extern GameManagerUnknownF0 g_MatchStages[17];
// A block GameManager's deleted callback clears (0x4d9698).
#ifdef TH09_EXE
// The popup storage is part of g_AsciiText, rather than a second object.
#define g_Unknown4d9698 (*(unsigned char (*)[0x2bc0])&g_AsciiText.popups)
#else
extern unsigned char g_Unknown4d9698[0x2bc0];
#endif

// Its update and draw chain elements (0x4a8120, 0x4a8140).
extern ChainElem g_GameManagerCalcChainElem;
extern ChainElem g_GameManagerDrawChainElem;

// Removes the GameManager's chain elements, caps both sides' score at
// 999999999 and copies it to guiScore, and resets gameSpeed, as th08's
// (0x41a827).
void RemoveGameManagerChain();

// Writes today's date to `buf` as "yy/mm/dd" (0x41a796).
void FormatTodayDate(char *buf);
