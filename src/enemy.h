// The enemies, one manager per side ("EnemyCtrlInf").

#pragma once

#include "bullet.h"
#include "chain.h"
#include "sprite.h"
#include "timer.h"

class EnemyManager;
struct GameSide;
struct Enemy;
struct EnemyUnknown22c;

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
    // Starts subroutine `sub` in `ctx`, unless it is negative, as th08's.
    // Returns 0 (0x406850).
    int StartSub(EnemyUnknown22c *ctx, short sub);

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
    // The subroutines, after the timelines' offsets in the file.
    void **subs;
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

// An argument of an ECL instruction, as th08's.
union EclArg
{
    int i;
    float f;
};

// An ECL instruction. Only the fields known so far: the enemy script
// function instruction takes the function in args[0] and passes it the
// instruction.
struct EclInstr
{
    unsigned char pad_0[0xc];
    EclArg args[9];
};

// `id`, or the float variable it names, as th08's GetFloatOperand
// (0x4068a0).
float GetFloatOperand(Enemy *enemy, float id);
// The float variable `operand` names when bit `bit` of `mask` is set,
// as th08's (0x406210).
float *ResolveFloatOperand(Enemy *enemy, float *operand, WORD mask, int bit);
// The interpolation loops' linear function, as th08's: sets args[8] to
// t of the way from args[4] to args[5] (0x407450).
void ScriptMoveLinear(Enemy *enemy, EclInstr *instr, float t);
// The Hermite one, from args[4] to args[5] with the slopes args[6] and
// args[7], as th08's (0x4074a0).
void ScriptMoveHermite(Enemy *enemy, EclInstr *instr, float t);

// The state of an enemy's ECL script, as th08's EnemyScriptState (0x22c
// bytes). Only the fields known so far; its implicit constructor is at
// 0x4120d0.
struct EnemyUnknown22c
{
    // The manager whose subroutine runs, and where it is.
    EclManager *ecl;
    void *code;
    // The frame of the subroutine, and of a wait.
    Timer timer;
    unsigned char pad_14[0x1c - 0x14];
    // The script's variables, by their place in th08's.
    int intVars[8];
    float floatVars[8];
    int intVars2[4];
    float floatVars2[2];
    unsigned char pad_74[0x94 - 0x74];
    Timer waitTimer;
    EnemyUnknown30 unknownA0[8];
    unsigned char pad_220[0x228 - 0x220];
    // The subroutine.
    short sub;
    unsigned char pad_22a[0x22c - 0x22a];
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
        this->bulletSound = -1;
    }

    // Sets command `i` to `flag`, running with the others if `concurrent`,
    // and adds the flag to flags. Returns the command (0x412650).
    BulletCommand *SetCommand(int i, int concurrent, DWORD flag);
    // Sets command `i` to flag 0x20, as th08's: turning by `arg0`
    // towards `arg1` for `frames` (0x412700).
    void SetAngleAccelCommand(int i, int concurrent, int frames, float arg1, float arg0);
    // Bullet::SetAccelCommand for the shooter (0x412730).
    void SetAccelCommand(int i, int concurrent, int frames, float arg0, float arg1);
    // Bullet::SetCommand20000 for the shooter (0x412760).
    void SetCommand20000(int i, int concurrent, int frames);

    // The bullet type, and its color, as th08's BulletShooter.
    short sprite;
    short color;
    D3DXVECTOR3 pos;
    float angle1;
    float angle2;
    float speed1;
    float speed2;
    // Copied into each bullet.
    BulletCommand commands[18];
    unsigned char pad_1d0[0x1f4 - 0x1d0];
    short count1;
    short count2;
    WORD aimMode;
    WORD unknown1fa;
    // The flags of the commands it has.
    DWORD flags;
    // The sounds of shooting and of the bullets, as th08's.
    int sound;
    int bulletSound;
    unsigned char pad_208[0x20c - 0x208];
    char unknown20c;
    unsigned char pad_20d[0x214 - 0x20d];
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
    // Resets the rank speeds and counts, as th08's inline (0x40f180).
    void ResetRankSpeeds();
    // Fades out and forgets the effects, as th08's ReleaseEnemyEffects
    // (0x40f560).
    void ReleaseEffects();
    // Frees the blocks of unknown33d8 (0x4100e0).
    void FreeUnknown33d8();

    EnemyManager *manager;
    unsigned char pad_4[0x8 - 0x4];
    Sprite primarySprite;
    Sprite sprites[2];
    EnemyUnknown22c unknown7f4;
    EnemyUnknown22c unknownA20[0x10];
    // The context running, as th08's currentCtx.
    EnemyUnknown22c *currentCtx;
    unsigned char pad_2ce4[0x2d2a - 0x2ce4];
    short stackDepth;
    unsigned char pad_2d2c[0x2d2e - 0x2d2c];
    short deathSub;
    unsigned char pad_2d30[0x2d70 - 0x2d30];
    short pendingInterrupt;
    unsigned char pad_2d72[0x2d74 - 0x2d72];
    D3DXVECTOR3 pos;
    unsigned char pad_2d80[0x2d8c - 0x2d80];
    D3DXVECTOR3 velocity;
    unsigned char pad_2d98[0x2dbc - 0x2d98];
    D3DXVECTOR3 hitboxSize;
    unsigned char pad_2dc8[0x2dd4 - 0x2dc8];
    D3DXVECTOR3 unknown2dd4;
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
    short rankCount1Low;
    short rankCount1High;
    short rankCount2Low;
    short rankCount2High;
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
    struct
    {
        DWORD unusedBits0 : 6;
        DWORD unknown3380 : 3;
        DWORD unusedBits9 : 1;
        // What EnemyManager::FindByUnknownType looks for.
        DWORD unknownType : 2;
    };
    unsigned char pad_3384[0x338a - 0x3384];
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
    // Effects that follow it, as th08's.
    struct Effect *effects[0x18];
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
    // The first active enemy, of the first 0x80, whose unknownType is
    // `type`, or NULL (0x40f7d0).
    Enemy *FindByUnknownType(int type);
    // Starts the first free enemy as subroutine `sub` (of playerEcl if
    // `usePlayerEcl`) at `pos`, mirrored if `mirrored`, with `life` and
    // `score` unless negative. Returns it (0x40f1d0).
    Enemy *SpawnEnemy(int sub, D3DXVECTOR3 *pos, int life, char itemDrop, int score, int mirrored,
                      int usePlayerEcl);

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
    unsigned char pad_2ac388[0x2ac3b8 - 0x2ac388];
    int unknown2ac3b8;
    unsigned char pad_2ac3bc[0x2ac3c8 - 0x2ac3bc];
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

// The lesser of the two ways round between angles `from` and `to`,
// times `t`, added to the smaller of the two (0x40f5e0).
float LerpAngleShort(float from, float to, float t);
