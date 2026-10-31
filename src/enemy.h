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

// The arguments of a subroutine call, as th08's.
struct EclCallArgs
{
    int ints[4];
    float floats[4];
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
    // Runs `enemy`'s script, as th08's RunEnemy. Returns -1 when the
    // enemy is gone (0x4086c0).
    int RunEnemy(Enemy *enemy);
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
    // The arguments the next subroutine call passes, as th08's.
    EclCallArgs callArgs;
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

// `id`, or the int variable it names, as th08's GetIntOperand
// (0x405760).
int GetIntOperand(Enemy *enemy, int id);
// The int variable `operand` names when bit `bit` of `mask` is set,
// as th08's (0x405f20).
int *ResolveOperand(Enemy *enemy, int *operand, WORD mask, int bit);
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

// An ECL script's variables, which SpawnEnemyWithVars copies in (0x78
// bytes).
struct EclVars
{
    int intVars[8];
    float floatVars[8];
    int intVars2[4];
    float floatVars2[2];
    EclCallArgs args;
};
ASSERT_SIZE(EclVars, 0x78);

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
    union
    {
        EclVars vars;
        struct
        {
            // The script's variables, by their place in th08's.
            int intVars[8];
            float floatVars[8];
            int intVars2[4];
            float floatVars2[2];
            EclCallArgs args;
        };
    };
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
    // The command each bullet starts from, as th08's.
    int commandIndex;
    // Copied into each bullet's unknown10bd.
    char unknown20c;
    unsigned char pad_20d[0x210 - 0x20d];
    // The bullet type's sprites, as th08's.
    BulletTypeSprites *typeSprites;
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
    // Stops the enemy unless it has a death mode, frees its boss slot,
    // effects and blocks and clears its callbacks, as th08's (0x410110).
    void Despawn();
    // Keeps pos within the bounds while clampPos is set, as th08's
    // (0x4101d0).
    void ClampPos();
    // Moves the enemy by its velocity (or velocity2) times the game
    // speed in x and y, as th08's (0x40f9b0).
    void Move();

    EnemyManager *manager;
    unsigned char pad_4[0x8 - 0x4];
    Sprite primarySprite;
    Sprite sprites[2];
    EnemyUnknown22c unknown7f4;
    EnemyUnknown22c unknownA20[0x10];
    // The context running, as th08's currentCtx.
    EnemyUnknown22c *currentCtx;
    // Where calls save the context, as th08's callStack.
    EnemyUnknown22c *callStack;
    // The enemy's own variables, as th08's.
    int enemyInts[8];
    float enemyFloats[8];
    short savedStackDepth;
    short stackDepth;
    unsigned char pad_2d2c[0x2d2e - 0x2d2c];
    short deathSub;
    unsigned char pad_2d30[0x2d70 - 0x2d30];
    short pendingInterrupt;
    unsigned char pad_2d72[0x2d74 - 0x2d72];
    D3DXVECTOR3 pos;
    // As th08's: the script moves pos, drawn at absPos = pos + posOrigin.
    D3DXVECTOR3 posOrigin;
    D3DXVECTOR3 velocity;
    // The velocity Move uses instead while useVelocity2 is set.
    D3DXVECTOR3 velocity2;
    // As th08's.
    D3DXVECTOR3 prevPos;
    D3DXVECTOR3 posDelta;
    D3DXVECTOR3 hitboxSize;
    unsigned char pad_2dc8[0x2dd4 - 0x2dc8];
    D3DXVECTOR3 absPos;
    float angle;
    float angularVelocity;
    // As th08's.
    float orbitAngle;
    float orbitAngularVelocity;
    unsigned char pad_2df0[0x2df4 - 0x2df0];
    float speed;
    float acceleration;
    float orbitRadius;
    unsigned char pad_2e00[0x2e04 - 0x2e00];
    D3DXVECTOR3 shootOffset;
    // A timed move, as th08's: by moveDelta from moveStart over
    // moveDuration frames.
    D3DXVECTOR3 moveDelta;
    D3DXVECTOR3 moveStart;
    Timer moveTimer;
    int moveDuration;
    // The range the bullet speed is scaled by with the rank, as th08's.
    float rankSpeedLow;
    float rankSpeedHigh;
    short rankCount1Low;
    short rankCount1High;
    short rankCount2Low;
    short rankCount2High;
    int life;
    // As th08's.
    int maxLife;
    int phaseStartLife;
    int score;
    // Its index in EnemyManager::enemies.
    int index;
    unsigned char pad_2e5c[0x2e64 - 0x2e5c];
    Timer callbackTimer;
    // The primary sprite's color when it spawned, as th08's.
    D3DCOLOR color;
    EnemyShooter shooter;
    unsigned char pad_3088[0x30b4 - 0x3088];
    int shootInterval;
    Timer shootTimer;
    EnemyShooter unknown30c4;
    unsigned char pad_32d8[0x3358 - 0x32d8];
    int laserIndex;
    // The item it drops, as th08's.
    int item;
    // Set by the timeline's opcodes 11 and 12, as th08's.
    int pointItems;
    int powerItems;
    BYTE deathEffect;
    BYTE deathEffectIndex;
    BYTE deathEffectVariant;
    // Its slot in EnemyManager::bosses, as th08's.
    BYTE bossIndex;
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
            // Set by SpellCard::Start on the other side's enemy of
            // unknownType 3.
            DWORD unknownFlag8 : 1;
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
        DWORD unusedBits0 : 3;
        // Not cleared for the bonus, as th08's.
        DWORD undying : 1;
        DWORD unusedBits4 : 2;
        DWORD unknown3380 : 3;
        // Set by the timeline's opcode 17.
        DWORD unknownBit9 : 1;
        // What EnemyManager::FindByUnknownType looks for.
        DWORD unknownType : 2;
        // Has Move use velocity2, unmirrored.
        DWORD useVelocity2 : 1;
    };
    unsigned char pad_3384[0x338a - 0x3384];
    short turnStill;
    unsigned char pad_338c[0x3390 - 0x338c];
    short turnLeft;
    short turnRight;
    unsigned char pad_3394[0x3398 - 0x3394];
    // Where ClampPos keeps it, as th08's.
    float lowerBound[2];
    float upperBound[2];
    float minShootDistSq;
    // Where th08's lastDamage is, which ECL reads as variable 10083.
    int lastDamage;
    int lifeCallbackLife[4];
    unsigned char pad_33c0[0x33d0 - 0x33c0];
    int timerCallbackTime;
    unsigned char pad_33d4[0x33d8 - 0x33d4];
    void *unknown33d8[4];
    EnemyUnknown1c trailBoxes[0x60];
    EnemyUnknown1c unknown3e68[0xc2];
    // As th08's: whether it has trail boxes, and how many.
    BYTE trailFlags;
    unsigned char pad_53a1[0x1];
    short trailSegmentCount;
    unsigned char pad_53a4[0x53a8 - 0x53a4];
    Timer unknown53a8;
    // Effects that follow it, as th08's.
    struct Effect *effects[0x18];
    int effectCount;
    unsigned char pad_5418[0x5424 - 0x5418];
    Timer unknown5424;
};
ASSERT_SIZE(Enemy, 0x5430);

// The spawn arguments of the timeline's opcodes, as th08's.
struct EnemyTimelineSpawn
{
    int sub;
    float x;
    float y;
    int life;
    int item;
    int score;
};
struct EnemyTimelineSpawnRandomX
{
    int sub;
    float xMin;
    float xMax;
    float y;
    int life;
    int item;
    int score;
};
struct EnemyTimelineSpawnAnyX
{
    int sub;
    float y;
    int life;
    int item;
    int score;
};
struct EnemyTimelineSpawnWithArgs
{
    int sub;
    float x;
    float y;
    int life;
    int pointItems;
    int powerItems;
    int score;
};

// One instruction of a timeline, as th08's: from `time` on, `opcode`
// runs on the difficulties in the bits of `difficulties`.
struct EnemyTimelineEntry
{
    int time;
    short opcode;
    BYTE size;
    BYTE difficulties;
    int args[1];
};

class EnemyManager;

// The side's timeline. Its implicit constructor is at 0x4120a0.
struct EnemyTimeline
{
    // Runs the instructions due, as th08's RunTimeline, then advances
    // timer by GameManager's timelineSpeed. Waits while a dialogue runs
    // and until delay runs out. Returns 1 once the timeline ends, else 0
    // (0x40fcc0).
    int Run();

    Timer timer;
    // Counts down before the timeline starts.
    Timer delay;
    // Whether spawns are mirrored; set by the mirrored opcodes and kept.
    int mirror;
    EnemyManager *mgr;
    EnemyTimelineEntry *pos;
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
    // Kills the enemies but the bosses and the undying, with a bonus
    // popup from 2000 up by 30 (to `maxBonus`) at each grazeable one and
    // its trail boxes, and starts their death subroutines, as th08's.
    // Returns `start` plus the bonuses (0x40f640).
    int ClearEnemiesForBonus(int maxBonus, int start);
    // Starts the first free enemy as subroutine `sub` (of playerEcl if
    // `usePlayerEcl`) at `pos`, mirrored if `mirrored`, with `life` and
    // `score` unless negative. Returns it (0x40f1d0).
    Enemy *SpawnEnemy(int sub, D3DXVECTOR3 *pos, int life, int itemDrop, int score, int mirrored,
                      int usePlayerEcl);
    // SpawnEnemy from playerEcl. Inline; the copy is at 0x440c80.
    Enemy *SpawnPlayerEnemy(int sub, D3DXVECTOR3 *pos, int life, int itemDrop, int score, int mirrored)
    {
        return this->SpawnEnemy(sub, pos, life, itemDrop, score, mirrored, 1);
    }
    // SpawnEnemy unmirrored, with the script's variables from `vars`, as
    // th08's SpawnWithVars (0x40f340).
    Enemy *SpawnEnemyWithVars(int sub, D3DXVECTOR3 *pos, int life, char itemDrop, int score, EclVars *vars,
                              int usePlayerEcl);
    // SpawnEnemyWithVars from playerEcl. Inline; the copy is at 0x4417c0.
    Enemy *SpawnPlayerEnemyWithVars(int sub, D3DXVECTOR3 *pos, int life, char itemDrop, int score, EclVars *vars)
    {
        return this->SpawnEnemyWithVars(sub, pos, life, itemDrop, score, vars, 1);
    }

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
    // The bosses, as th08's.
    Enemy *bosses[4];
    unsigned char pad_2ac398[0x2ac3b8 - 0x2ac398];
    int unknown2ac3b8;
    unsigned char pad_2ac3bc[0x2ac3c8 - 0x2ac3bc];
    Timer unknown2ac3c8;
    unsigned char unknown2ac3d4;
    unsigned char pad_2ac3d5[0x2ac3e0 - 0x2ac3d5];
    // The timeline running.
    EnemyTimeline timeline;
    Timer unknown2ac404;
    // The enemies drawn by OnDraw (0 and 1) and OnDraw2 (2 and 3), as
    // th08's.
    Enemy *drawListHeads[4];
    // enemy.anm (or the stage's variant), slot 9.
    AnmLoaded *anm;
    // The other side's player ANM.
    AnmLoaded *otherPlayerAnm;
    unsigned char pad_2ac428[0x2ac42c - 0x2ac428];
    // Whether the last spawn found no free enemy, as th08's.
    int spawnFailed;
    // The timeline's signals, as th08's.
    int timelineSignals[4];
    unsigned char pad_2ac440[0x2ac44c - 0x2ac440];
    // Set by AddedCallback: 200 if the side's character is 5, else 160.
    int unknown2ac44c;
};
ASSERT_SIZE(EnemyManager, 0x2ac450);

// The order of the timelines, shared by the sides: an index in the low
// seven bits, and a random bit 7 (0x4a7c88).
extern unsigned char g_TimelineOrder[0x100];

// The lesser of the two ways round between angles `from` and `to`,
// times `t`, added to the smaller of the two (0x40f5e0).
float LerpAngleShort(float from, float to, float t);
