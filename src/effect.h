// The effects, one manager per side ("EffectSysInf").

#pragma once

#include "chain.h"
#include "timer.h"

struct GameSide;
class AnmLoaded;

// An effect (0xd8 bytes). Only the fields known so far.
struct Effect
{
    unsigned char pad_0[0x4];
    void *unknown04;
    unsigned char pad_8[0xac - 0x8];
    Timer unknownAc;
    unsigned char pad_b8[0xcc - 0xb8];
    void *unknownCc;
    unsigned char pad_d0[0xd4 - 0xd0];
    // The next effect of a list.
    Effect *next;
};
ASSERT_SIZE(Effect, 0xd8);

// A side's effect manager (0x2dc bytes); the third side has one too. Only
// the fields known so far.
class EffectManager
{
public:
    // Zeroes the manager. Inline; the copy is at 0x40c950.
    void Init()
    {
        memset(this, 0, sizeof(EffectManager));
    }
    // Zeroes the manager, then sets unknown20 to 1. Inline; the copy is
    // at 0x40d2e0.
    EffectManager()
    {
        this->Init();
        this->unknown20[0] = 1.0f;
        this->unknown20[1] = 1.0f;
        this->unknown20[2] = 1.0f;
        this->unknown20[3] = 1.0f;
    }

    // Frees the blocks of each effect and the effects. Returns 0. Clears
    // unknownCc where it should clear unknown04 (0x40d200).
    static int FreeEffects(EffectManager *mgr);
    // Frees the effects, removes the manager's chain elements and deletes
    // it, if any (0x40d280).
    static void RemoveChain(EffectManager *mgr);
    // Resets the effects for a new round (0x40c990).
    void Reset();
    // The update and draw chain callbacks; the third side's draws with
    // OnDraw2 (0x40cdd0, 0x40cfb0, 0x40d070).
    static int OnUpdate(EffectManager *mgr);
    static int OnDraw(EffectManager *mgr);
    static int OnDraw2(EffectManager *mgr);
    // The update element's added callback: takes etama.anm from ANM
    // slot 8, loading it if needed. Returns 0 (0x40d1c0).
    static int AddedCallback(EffectManager *mgr);
    // Creates side `sideIdx`'s effect manager with `count34` + `count38`
    // + 1 effects and adds its update element (priority 0x13 +
    // `sideIdx`) and draw element (the same, or 0x1e for the third
    // side). Returns it, or NULL if adding fails (0x40d320).
    static EffectManager *RegisterChain(int sideIdx, int count34, int count38);

    unsigned char pad_0[0xc];
    int sideIdx;
    // The side, and the other one; for the third side, both itself.
    GameSide *side;
    GameSide *otherSide;
    ChainElem *calcElem;
    ChainElem *drawElem;
    float unknown20[4];
    Effect *effects;
    // The effects are unknown34 + unknown38 + 1.
    int unknown34;
    int unknown38;
    // The heads of three lists of effects; OnDraw2 draws the first.
    Effect unknown3c;
    Effect unknown114;
    Effect unknown1ec;
    unsigned char pad_2c4[0x2d4 - 0x2c4];
    AnmLoaded *anm;
    unsigned char pad_2d8[0x2dc - 0x2d8];
};
ASSERT_SIZE(EffectManager, 0x2dc);
