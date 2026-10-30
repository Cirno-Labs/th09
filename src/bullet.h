// The bullets, one manager per side.

#pragma once

#include "chain.h"

class AnmLoaded;

// An enemy bullet (0x10c4 bytes). Only the fields known so far.
struct Bullet
{
    unsigned char pad_0[0xdbe];
    // 0 while the slot is free; 6 marks the spare bullet after the last,
    // as th08's.
    unsigned short state;
    unsigned char pad_dc0[0x10c4 - 0xdc0];
};
ASSERT_SIZE(Bullet, 0x10c4);

// A laser (0x59c bytes, th08's size).
struct Laser
{
    unsigned char pad_0[0x59c];
};
ASSERT_SIZE(Laser, 0x59c);

// A side's bullet manager, which loads etama.anm. Only the fields known
// so far.
class BulletManager
{
public:
    // Removes the manager's chain elements and deletes it, if any
    // (0x414180).
    static void RemoveChain(BulletManager *mgr);
    // Clears the bullets for a new round (0x412470).
    void Reset();

    unsigned char pad_0[0x1a900];
    // Two sets of bullets, each with a spare after the last.
    Bullet bullets[0xb0];
    Bullet bullets2[0x169];
    Laser lasers[0x30];
    unsigned char pad_25e164[0x25e184 - 0x25e164];
    ChainElem *calcElem;
    ChainElem *drawElem;
    unsigned char pad_25e18c[0x25e1b0 - 0x25e18c];
    // Where the searches for a free bullet start, as th08's nextBullet.
    Bullet *nextBullet;
    Bullet *nextBullet2;
    // The item type cleared bullets turn into, as th08's.
    int clearItemType;
    AnmLoaded *etamaAnm;
};
