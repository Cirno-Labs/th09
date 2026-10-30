// The bullets, one manager per side.

#pragma once

#include "chain.h"
#include "sprite.h"
#include "timer.h"

struct EnemyShooter;
struct GameSide;

// The five sprites of a bullet type, as th08's (0xd48 bytes). Only the
// fields known so far.
struct BulletTypeSprites
{
    Sprite sprite0;
    Sprite sprite1;
    Sprite sprite2;
    Sprite sprite3;
    Sprite sprite4;
    unsigned char pad_d34[0xd48 - 0xd34];
};
ASSERT_SIZE(BulletTypeSprites, 0xd48);

// A bullet's extra behavior state, as th08's BulletEx (0x2c bytes). Only
// the fields known so far.
struct BulletEx
{
    Timer timer;
    unsigned char pad_c[0x2c - 0xc];
};
ASSERT_SIZE(BulletEx, 0x2c);

// A step of a bullet's behavior, as th08's (0x18 bytes).
struct BulletCommand
{
    float arg0;
    float arg1;
    // Frames, for most flags.
    int frames;
    int maxTurns;
    DWORD flag;
    int concurrentFlag;
};
ASSERT_SIZE(BulletCommand, 0x18);

// An enemy bullet (0x10c4 bytes). Only the fields known so far.
struct Bullet
{
    // Sets command `i` to `flag`, running with the others if `concurrent`,
    // adds the flag to flags and starts over from the first command.
    // Returns the command (0x412610).
    BulletCommand *SetCommand(int i, int concurrent, DWORD flag);
    // Sets command `i` to flag 0x10, as th08's: speeding up by `arg0`
    // towards `arg1` for `frames` (0x412680).
    void SetAccelCommand(int i, int concurrent, int frames, float arg0, float arg1);
    // Sets command `i` to flag 0x20000, which runs for `frames`, as
    // th08's (0x4126b0).
    void SetCommand20000(int i, int concurrent, int frames);
    // Sets command `i` to flag 0x4000, which switches the bullet to
    // bullet type `type` and color `color`, as th08's (0x4126d0).
    void SetSpriteCommand(int i, int concurrent, int type, int color);
    // Clears the commands, their flags and unknownDb4, and starts over
    // from the first command (0x40c390).
    void ClearCommands();
    // Frees the bullet: state 0 and both timers reset, as th08's
    // (0x412930).
    void FreeBullet();

    unsigned char pad_0[0x4];
    BulletTypeSprites sprites;
    unsigned char pad_d4c[0xd88 - 0xd4c];
    Timer timer;
    Timer grazeDelayTimer;
    unsigned char pad_da0[0xdb4 - 0xda0];
    int unknownDb4;
    // The flags of the commands it has.
    DWORD flags;
    unsigned char pad_dbc[0xdbe - 0xdbc];
    // 0 while the slot is free; 6 marks the spare bullet after the last,
    // as th08's.
    unsigned short state;
    unsigned char pad_dc0[0xdd4 - 0xdc0];
    // The command to start next, as th08's.
    int commandIndex;
    BulletCommand commands[18];
    BulletEx ex[7];
    unsigned char pad_10bc[0x10c4 - 0x10bc];
};
ASSERT_SIZE(Bullet, 0x10c4);

// A laser, as th08's (0x59c bytes). Only the fields known so far.
struct Laser
{
    Sprite sprite0;
    Sprite sprite1;
    unsigned char pad_548[0x588 - 0x548];
    Timer timer;
    unsigned char pad_594[0x59c - 0x594];
};
ASSERT_SIZE(Laser, 0x59c);

// A side's bullet manager, which loads etama.anm. Only the fields known
// so far.
class BulletManager
{
public:
    // Constructs the members and initializes (0x415050).
    BulletManager();

    // Calls Unknown412590 with 1. Called by the ECL interpreter; the
    // copy is at 0x405650.
    void Unknown405650();
    // Clears unknown25e198 (0x412390).
    void ClearUnknown25e198();
    // Zeroes everything, marks the spare bullets and clears the script
    // index of every bullet's sprites, as th08's Reset (0x4123c0).
    void Initialize();
    // Removes the manager's chain elements and deletes it, if any
    // (0x414180).
    static void RemoveChain(BulletManager *mgr);
    // Clears the bullets for a new round (0x412470).
    void Reset();
    // Empty; called by the ECL interpreter (0x412590).
    void Unknown412590(int arg);
    // Empty; called by the ECL interpreter (0x4125a0).
    void Unknown4125a0(int arg, int arg2);
    // Preloads etama.anm into ANM slot 8; returns 1 if that fails
    // (0x4125e0).
    static int LoadAnm();
    // Releases ANM slot 8 (0x412600).
    static void ReleaseAnm();
    // Fires the shooter's count1 by count2 bullets, as th08's
    // FirePattern. Returns the last bullet spawned (0x4130f0).
    int FirePattern(EnemyShooter *shooter);
    // The update chain callback and its added callback, and the draw
    // chain callback (0x4146f0, 0x413e40, 0x413be0).
    static int OnUpdate(BulletManager *mgr);
    static int AddedCallback(BulletManager *mgr);
    static int OnDraw(BulletManager *mgr);
    // Creates side `sideIdx`'s bullet manager and adds its update element
    // (priority 0xc + `sideIdx`) and draw element (0x15 + `sideIdx`).
    // Returns it, or NULL if adding fails (0x4150d0).
    static BulletManager *RegisterChain(int sideIdx);

    BulletTypeSprites bulletTypes[0x20];
    // Two sets of bullets, each with a spare after the last.
    Bullet bullets[0xb0];
    Bullet bullets2[0x169];
    Laser lasers[0x30];
    unsigned char pad_25e164[0x25e174 - 0x25e164];
    Timer unknown25e174;
    unsigned char pad_25e180[0x25e184 - 0x25e180];
    ChainElem *calcElem;
    ChainElem *drawElem;
    int sideIdx;
    // The side, and the other one.
    GameSide *side;
    GameSide *otherSide;
    // Cleared by ClearUnknown25e198.
    int unknown25e198[6];
    // Where the searches for a free bullet start, as th08's nextBullet.
    Bullet *nextBullet;
    Bullet *nextBullet2;
    // The item type cleared bullets turn into, as th08's.
    int clearItemType;
    AnmLoaded *etamaAnm;
};
ASSERT_SIZE(BulletManager, 0x25e1c0);
