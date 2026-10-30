// The enemies, one manager per side ("EnemyCtrlInf").

#pragma once

#include "chain.h"
#include "sprite.h"
#include "timer.h"

class EnemyManager;
struct GameSide;

// The head of an ECL file, as th08's: then the offsets of the timelines.
struct EclHeader
{
    int version;
    short subCount;
    short timelineCount;
    void *timelines[1];
};

// The state of an ECL script. Only the fields known so far; its
// implicit constructor is at 0x40fca0.
struct EclContext
{
    unsigned char pad_0[0x100];
    D3DXVECTOR3 unknown100[8];
};

// An ECL file and the state of its scripts (0x188 bytes). Only the
// fields known so far; its implicit constructor is at 0x412090.
struct EclManager
{
    // Loads the ECL file `path`. Returns 0, or -1 on failure (0x405660).
    int Load(char *path);
    // Frees the ECL file (0x406830).
    void Unload();

    // How many timelines the file has. Inline, as th08's; the copy is
    // at 0x40f160.
    int GetTimelineCount()
    {
        return this->file->timelineCount;
    }
    // Timeline `i` of the file. Inline, as th08's; the copy is at
    // 0x40f170.
    void *GetTimeline(int i)
    {
        return this->file->timelines[i];
    }

    EclHeader *file;
    unsigned char pad_4[0x8 - 0x4];
    EclContext context;
    unsigned char pad_188[0x188 - 0x8 - sizeof(EclContext)];
};
ASSERT_SIZE(EclManager, 0x188);

// Something of 0x30 bytes with a Timer at 4. Only the fields known so
// far; its implicit constructor is at 0x4120c0.
struct EnemyUnknown30
{
    unsigned char pad_0[0x4];
    Timer timer;
    unsigned char pad_10[0x30 - 0x10];
};
ASSERT_SIZE(EnemyUnknown30, 0x30);

// Something of 0x22c bytes. Only the fields known so far; its implicit
// constructor is at 0x4120d0.
struct EnemyUnknown22c
{
    unsigned char pad_0[0x8];
    Timer unknown8;
    unsigned char pad_14[0x94 - 0x14];
    Timer unknown94;
    EnemyUnknown30 unknownA0[8];
    unsigned char pad_220[0x22c - 0x220];
};
ASSERT_SIZE(EnemyUnknown22c, 0x22c);

// How an enemy shoots, as th08's shooter (0x214 bytes). Only the fields
// known so far.
struct EnemyShooter
{
    // Inline; the copy is at 0x40d500.
    EnemyShooter()
    {
        memset(this, 0, sizeof(EnemyShooter));
        this->unknown204 = -1;
    }

    unsigned char pad_0[0x200];
    // The sounds of shooting and of the bullets, as th08's.
    int sound;
    int unknown204;
    unsigned char pad_208[0x214 - 0x208];
};
ASSERT_SIZE(EnemyShooter, 0x214);

// Something of 0x1c bytes, starting with a position, with an empty
// constructor; the linker folded it with D3DXVECTOR3's (0x4343d0).
struct EnemyUnknown1c
{
    EnemyUnknown1c()
    {
    }

    D3DXVECTOR3 pos;
    unsigned char pad_c[0x1c - 0xc];
};
ASSERT_SIZE(EnemyUnknown1c, 0x1c);

// An enemy (0x5430 bytes), as th08's as far as EnemyManager::Initialize
// shows it. Only the fields known so far.
struct Enemy
{
    // Frees the blocks of unknown33d8 (0x4100e0).
    void FreeUnknown33d8();

    EnemyManager *manager;
    unsigned char pad_4[0x8 - 0x4];
    Sprite primarySprite;
    Sprite sprites[2];
    EnemyUnknown22c unknown7f4;
    EnemyUnknown22c unknownA20[0x10];
    unsigned char pad_2ce0[0x2d2a - 0x2ce0];
    short stackDepth;
    unsigned char pad_2d2c[0x2d2e - 0x2d2c];
    short deathSub;
    unsigned char pad_2d30[0x2d70 - 0x2d30];
    short pendingInterrupt;
    unsigned char pad_2d72[0x2d8c - 0x2d72];
    D3DXVECTOR3 velocity;
    unsigned char pad_2d98[0x2dbc - 0x2d98];
    D3DXVECTOR3 hitboxSize;
    unsigned char pad_2dc8[0x2de0 - 0x2dc8];
    float angle;
    float angularVelocity;
    unsigned char pad_2de8[0x2df4 - 0x2de8];
    float speed;
    float acceleration;
    unsigned char pad_2dfc[0x2e04 - 0x2dfc];
    D3DXVECTOR3 shootOffset;
    unsigned char pad_2e10[0x2e28 - 0x2e10];
    Timer unknown2e28;
    unsigned char pad_2e34[0x2e38 - 0x2e34];
    // The range the bullet speed is scaled by with the rank, as th08's.
    float rankSpeedLow;
    float rankSpeedHigh;
    unsigned char pad_2e40[0x2e48 - 0x2e40];
    int life;
    unsigned char pad_2e4c[0x2e54 - 0x2e4c];
    int score;
    unsigned char pad_2e58[0x2e64 - 0x2e58];
    Timer callbackTimer;
    unsigned char pad_2e70[0x2e74 - 0x2e70];
    EnemyShooter shooter;
    unsigned char pad_3088[0x30b4 - 0x3088];
    int shootInterval;
    Timer shootTimer;
    EnemyShooter unknown30c4;
    unsigned char pad_32d8[0x3358 - 0x32d8];
    int laserIndex;
    unsigned char pad_335c[0x3368 - 0x335c];
    BYTE deathEffect;
    BYTE deathEffectIndex;
    BYTE deathEffectVariant;
    unsigned char pad_336b[0x336c - 0x336b];
    BYTE hitFlashTimer;
    unsigned char pad_336d[0x3370 - 0x336d];
    Timer unknown3370;
    // The flags, as th08's but with fewer of them.
    union
    {
        DWORD flags;
        struct
        {
            DWORD active : 1;
            DWORD isBoss : 1;
            DWORD shootable : 1;
            DWORD contactDamage : 1;
            DWORD hideFollowEffects : 1;
            DWORD unusedFlag5 : 1;
            DWORD hitByShots : 1;
            DWORD grazeable : 1;
            DWORD unusedFlag8 : 1;
            DWORD moveMode : 2;
            DWORD moveEasing : 3;
            DWORD holdFire : 1;
            DWORD mirrored : 1;
            DWORD clampPos : 1;
            DWORD deathMode : 3;
            DWORD unusedFlag20 : 1;
            DWORD onScreen : 1;
            DWORD rotateSprite : 1;
        };
    };
    unsigned char pad_3380[0x338a - 0x3380];
    short turnStill;
    unsigned char pad_338c[0x3390 - 0x338c];
    short turnLeft;
    short turnRight;
    unsigned char pad_3394[0x33a8 - 0x3394];
    float minShootDistSq;
    unsigned char pad_33ac[0x33b0 - 0x33ac];
    int lifeCallbackLife[4];
    unsigned char pad_33c0[0x33d0 - 0x33c0];
    int timerCallbackTime;
    unsigned char pad_33d4[0x33d8 - 0x33d4];
    void *unknown33d8[4];
    EnemyUnknown1c trailBoxes[0x60];
    EnemyUnknown1c unknown3e68[0xc2];
    unsigned char pad_53a0[0x53a8 - 0x53a0];
    Timer unknown53a8;
    unsigned char pad_53b4[0x5414 - 0x53b4];
    int effectCount;
    unsigned char pad_5418[0x5424 - 0x5418];
    Timer unknown5424;
};
ASSERT_SIZE(Enemy, 0x5430);

// Two Timers of the timeline. Its implicit constructor is at
// 0x4120a0.
struct EnemyTimelineTimers
{
    Timer unknown0;
    Timer unknownC;
};

// A side's enemy manager (0x2ac450 bytes). Only the fields known so far.
class EnemyManager
{
public:
    // Constructs the members and initializes (0x4121d0).
    EnemyManager();

    // Zeroes everything and sets up enemyTemplate, as th08's (0x40fa70).
    void Initialize();
    // Frees the enemies' resources, removes the manager's chain elements
    // and deletes it, if any (0x411f80).
    static void RemoveChain(EnemyManager *mgr);
    // Resets the enemies for a new round (0x411fd0).
    void Reset();
    // The update chain callback and its added callback, and the two draw
    // chain callbacks (0x410730, 0x411db0, 0x411d50, 0x411d80).
    static int OnUpdate(EnemyManager *mgr);
    static int AddedCallback(EnemyManager *mgr);
    static int OnDraw(EnemyManager *mgr);
    static int OnDraw2(EnemyManager *mgr);
    // Creates side `sideIdx`'s enemy manager and adds its update element
    // (priority 0xa + `sideIdx`) and draw elements (0xa + `sideIdx` and
    // 0x11 + `sideIdx`). Returns it, or NULL if adding fails (0x412240).
    static EnemyManager *RegisterChain(int sideIdx);

    // enemy.ecl, and the character's plXX.ecl.
    EclManager ecl;
    EclManager playerEcl;
    ChainElem *calcElem;
    ChainElem *drawElem;
    ChainElem *drawElem2;
    // Only for side 0 does Reset roll g_TimelineOrder.
    int sideIdx;
    // The side, and the other one.
    GameSide *side;
    GameSide *otherSide;
    // What Initialize resets each enemy from, as th08's.
    Enemy enemyTemplate;
    // The enemies, and one more.
    Enemy enemies[0x81];
    unsigned char pad_2ac388[0x2ac3c8 - 0x2ac388];
    Timer unknown2ac3c8;
    unsigned char unknown2ac3d4;
    unsigned char pad_2ac3d5[0x2ac3e0 - 0x2ac3d5];
    EnemyTimelineTimers timelineTimers;
    unsigned char pad_2ac3f8[0x2ac400 - 0x2ac3f8];
    // The timeline running.
    void *timeline;
    Timer unknown2ac404;
    unsigned char pad_2ac410[0x2ac430 - 0x2ac410];
    // The timeline's signals, as th08's.
    int timelineSignals[4];
    unsigned char pad_2ac440[0x2ac450 - 0x2ac440];
};
ASSERT_SIZE(EnemyManager, 0x2ac450);

// The order of the timelines, shared by the sides: an index in the low
// seven bits, and a random bit 7 (0x4a7c88).
extern unsigned char g_TimelineOrder[0x100];
