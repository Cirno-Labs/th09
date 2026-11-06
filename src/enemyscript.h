// The enemy scripts (ECL): the file, its interpreter and a script's state, as th06's.
// Included by enemy.h where these declarations were; it relies on what
// enemy.h declares before it.

#pragma once

// The head of an ECL file, as th08's: then the offsets of the timelines.
struct EnemyScriptHeader
{
    int version;
    short subCount;
    short timelineCount;
    void *timelines[1];
};

// The arguments of a subroutine call, as th08's.
struct EnemyScriptCallArgs
{
    int ints[4];
    float floats[4];
};

// th06's ScriptStarPattern keeps its star here; th09 only constructs
// it. Only the fields known so far; its implicit constructor is at
// 0x40fca0.
struct EnemyScriptsStar
{
    unsigned char pad_0[0x100];
    D3DXVECTOR3 ends[8];
};

// An ECL file and the state of its scripts (0x188 bytes). Only the
// fields known so far; its implicit constructor is at 0x412090.
struct EnemyScripts
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
    int StartSub(EnemyScriptState *ctx, short sub);

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

    EnemyScriptHeader *file;
    // The subroutines, after the timelines' offsets in the file.
    void **subs;
    EnemyScriptsStar star;
    // The arguments the next subroutine call passes, as th08's.
    EnemyScriptCallArgs callArgs;
};
ASSERT_SIZE(EnemyScripts, 0x188);

// An interpolation loop of an ECL script, as th08's EnemyScriptLoop:
// runs `func` for `duration` frames with the eased time, to set the
// variable `target` names. Its implicit constructor is at 0x4120c0.
struct EnemyScriptLoop
{
    void (*func)(Enemy *enemy, EnemyScriptInstr *instr, float t);
    Timer timer;
    int duration;
    int type;
    int mode;
    float args[4];
    float target;
};
ASSERT_SIZE(EnemyScriptLoop, 0x30);

// An argument of an ECL instruction, as th08's.
union EnemyScriptArg
{
    int i;
    float f;
};

// An ECL instruction, as th08's: the enemy script function instruction
// takes the function in args[0] and passes it the instruction.
struct EnemyScriptInstr
{
    // The frame it runs on.
    int time;
    short opcode;
    // The size of the instruction, arguments included.
    short size;
    BYTE pad_8;
    BYTE difficultyMask;
    // Bit n: argument n names a variable.
    WORD paramMask;
    EnemyScriptArg args[9];
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
void ScriptMoveLinear(Enemy *enemy, EnemyScriptInstr *instr, float t);
// The Hermite one, from args[4] to args[5] with the slopes args[6] and
// args[7], as th08's (0x4074a0).
void ScriptMoveHermite(Enemy *enemy, EnemyScriptInstr *instr, float t);

// The arguments of the spawn instructions, as th08's.
struct EnemySpawnArgs
{
    int sub;
    D3DXVECTOR3 pos;
    int life;
    int item;
    int score;
};
ASSERT_SIZE(EnemySpawnArgs, 0x1c);

// A function opcodes 136 and 137 run, as th08's EnemyScriptFunc.
typedef void (*EnemyScriptFunc)(Enemy *enemy, EnemyScriptInstr *instr);
// The interpolations by loop type, as th08's (0x4a0af4).
extern void (*g_EnemyScriptInterps[8])(Enemy *enemy, EnemyScriptInstr *instr, float t);
// The functions opcodes 136 and 137 run (0x4a0b14).
extern EnemyScriptFunc g_EnemyScriptFuncs[12];

// An ECL script's variables, which SpawnWithVars copies in (0x78
// bytes).
struct EnemyScriptVars
{
    int intVars[8];
    float floatVars[8];
    int intVars2[4];
    float floatVars2[2];
    EnemyScriptCallArgs args;
};
ASSERT_SIZE(EnemyScriptVars, 0x78);

// The state of an enemy's ECL script, as th08's EnemyScriptState (0x22c
// bytes). Only the fields known so far; its implicit constructor is at
// 0x4120d0.
struct EnemyScriptState
{
    // The manager whose subroutine runs, and where it is.
    EnemyScripts *ecl;
    void *code;
    // The frame of the subroutine, and of a wait.
    Timer timer;
    // What runs each frame with `funcInstr`, as th08's.
    void (*func)(Enemy *enemy, EnemyScriptInstr *instr);
    EnemyScriptInstr *funcInstr;
    union
    {
        EnemyScriptVars vars;
        struct
        {
            // The script's variables, by their place in th08's.
            int intVars[8];
            float floatVars[8];
            int intVars2[4];
            float floatVars2[2];
            EnemyScriptCallArgs args;
        };
    };
    Timer waitTimer;
    EnemyScriptLoop loops[8];
    int compareResult;
    // 1 + the index of the thread it is, or 0 for the main one.
    int timedCallActive;
    // The subroutine.
    short sub;
    unsigned char pad_22a[0x22c - 0x22a];
};
ASSERT_SIZE(EnemyScriptState, 0x22c);

// One of an enemy's four extra script threads, as th08's ("ECLInt"),
// which RunEnemy runs after its main context each frame.
struct EnemyScriptThread
{
    int sub;
    short pad_4;
    // Enemy::stackDepth while the thread runs.
    short stackDepth;
    EnemyScriptState ctx;
    // Its call stack (Enemy::callStack while it runs).
    EnemyScriptState callStack[16];
};
ASSERT_SIZE(EnemyScriptThread, 0x24f4);
