// The bullets, one manager per side.

#pragma once

#include "chain.h"
#include "sprite.h"
#include "timer.h"

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

// An enemy bullet (0x10c4 bytes). Only the fields known so far.
struct Bullet
{
    unsigned char pad_0[0x4];
    BulletTypeSprites sprites;
    unsigned char pad_d4c[0xd88 - 0xd4c];
    Timer timer;
    Timer grazeDelayTimer;
    unsigned char pad_da0[0xdbe - 0xda0];
    // 0 while the slot is free; 6 marks the spare bullet after the last,
    // as th08's.
    unsigned short state;
    unsigned char pad_dc0[0xf88 - 0xdc0];
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

    // Zeroes everything, marks the spare bullets and clears the script
    // index of every bullet's sprites, as th08's Reset (0x4123c0).
    void Initialize();
    // Removes the manager's chain elements and deletes it, if any
    // (0x414180).
    static void RemoveChain(BulletManager *mgr);
    // Clears the bullets for a new round (0x412470).
    void Reset();
    // Preloads etama.anm into ANM slot 8; returns 1 if that fails
    // (0x4125e0).
    static int LoadAnm();
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
    unsigned char pad_25e198[0x25e1b0 - 0x25e198];
    // Where the searches for a free bullet start, as th08's nextBullet.
    Bullet *nextBullet;
    Bullet *nextBullet2;
    // The item type cleared bullets turn into, as th08's.
    int clearItemType;
    AnmLoaded *etamaAnm;
};
ASSERT_SIZE(BulletManager, 0x25e1c0);
