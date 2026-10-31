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
    // The size of the bullet's hitbox.
    D3DXVECTOR3 hitboxSize;
    unsigned char pad_d40[0xd42 - 0xd40];
    // The draw list the bullet goes into, as th08's.
    BYTE sizeClass;
    unsigned char pad_d43[0xd44 - 0xd43];
    // 0x14a for the bullets that extra attack type 21 leaves alone.
    int unknownD44;
};
ASSERT_SIZE(BulletTypeSprites, 0xd48);

// A bullet's extra behavior state, as th08's BulletEx (0x2c bytes). Only
// the fields known so far.
struct BulletEx
{
    Timer timer;
    float turnSpeed;
    float turnAngle;
    // Added to the velocity (flag 0x10).
    D3DXVECTOR3 accel;
    // How long the behavior runs, in frames.
    int duration;
    int maxTurns;
    int turnCount;
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

class BulletManager;

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
    // Clears the commands, their flags and activeFlags, and starts over
    // from the first command (0x40c390).
    void ClearCommands();
    // Frees the bullet: state 0 and both timers reset, as th08's
    // (0x412930).
    void FreeBullet();

    // The manager the bullet belongs to.
    BulletManager *mgr;
    BulletTypeSprites sprites;
    // The motion, as th08's.
    D3DXVECTOR3 pos;
    D3DXVECTOR3 velocity;
    D3DXVECTOR3 accel;
    float speed;
    float accelSpeed;
    float turnSpeed;
    float angle;
    float angularVelocity;
    float turnAngle;
    Timer timer;
    Timer grazeDelayTimer;
    unsigned char pad_da0[0xdb0 - 0xda0];
    // Frames before the bounds check starts, as th08's.
    int boundsCheckDelay;
    // The flags of the commands running, as th08's.
    DWORD activeFlags;
    // The flags of the commands it has.
    DWORD flags;
    unsigned char pad_dbc[0xdbe - 0xdbc];
    // 0 while the slot is free; 6 marks the spare bullet after the last,
    // as th08's.
    unsigned short state;
    WORD outOfBoundsFrames;
    unsigned char pad_dc2[0xdc3 - 0xdc2];
    BYTE grazed;
    // Set while spawning in one of the player's hit areas; the bullet
    // fades out once spawned.
    BYTE spawnedInArea;
    unsigned char pad_dc5[0xdc8 - 0xdc5];
    // The next bullet in its draw list.
    Bullet *next;
    unsigned char pad_dcc[0xdd0 - 0xdcc];
    // The sound the commands play, if not negative.
    int sound;
    // The command to start next, as th08's.
    int commandIndex;
    BulletCommand commands[18];
    BulletEx ex[7];
    // Neither grazed nor hit, as th08's.
    char harmless;
    unsigned char pad_10bd[0x10be - 0x10bd];
    // Set once extra attack type 21 has doubled the bullet, and on the
    // copy.
    char unknown10be;
    unsigned char pad_10bf[0x10c4 - 0x10bf];
};
ASSERT_SIZE(Bullet, 0x10c4);

// Starts the bullet's next commands, as th08's (0x4141c0).
void RunBulletCommands(Bullet *bullet);

// A laser, as th08's (0x59c bytes). Only the fields known so far.
struct Laser
{
    Sprite sprite0;
    Sprite sprite1;
    D3DXVECTOR3 pos;
    float angle;
    float startOffset;
    float endOffset;
    float maxLength;
    float width;
    float baseWidth;
    float speed;
    int startFrames;
    int hitStartFrame;
    int durationFrames;
    int endFrames;
    int endHitFrames;
    int active;
    Timer timer;
    WORD flags;
    short color;
    BYTE state;
    // Keeps the start sprite hidden until the laser is past state 0.
    BYTE drawDelayFlag;
    unsigned char pad_59a[0x59c - 0x59a];
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
    // Clears unknown25e198. Inline: OnUpdate reloads its registers after
    // the copy at 0x412390.
    void ClearUnknown25e198()
    {
        this->unknown25e198[0] = this->unknown25e198[1] = this->unknown25e198[2] = this->unknown25e198[3] =
            this->unknown25e198[4] = this->unknown25e198[5] = 0;
    }
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
    Bullet *FirePattern(EnemyShooter *shooter);
    // FirePattern, spawning each bullet with an extra argument of 1
    // (0x4131c0).
    Bullet *FirePattern2(EnemyShooter *shooter);
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
    // The live bullets of each set, and of both.
    int bulletCount1;
    int bulletCount2;
    int bulletCount;
    // Counts down, as th08's clearAllFrames.
    int clearAllFrames;
    Timer updateTimer;
    int updateFrameCount;
    ChainElem *calcElem;
    ChainElem *drawElem;
    int sideIdx;
    // The side, and the other one.
    GameSide *side;
    GameSide *otherSide;
    // The bullets to draw, one list per size class; cleared by
    // ClearUnknown25e198.
    Bullet *unknown25e198[6];
    // Where the searches for a free bullet start, as th08's nextBullet.
    Bullet *nextBullet;
    Bullet *nextBullet2;
    // The item type cleared bullets turn into, as th08's.
    int clearItemType;
    AnmLoaded *etamaAnm;
};
ASSERT_SIZE(BulletManager, 0x25e1c0);
