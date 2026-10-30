// The effects, one manager per side ("EffectSysInf").

#pragma once

#include "chain.h"

// An effect (0xd8 bytes). Only the fields known so far.
struct Effect
{
    unsigned char pad_0[0x4];
    void *unknown04;
    unsigned char pad_8[0xcc - 0x8];
    void *unknownCc;
    unsigned char pad_d0[0xd8 - 0xd0];
};
ASSERT_SIZE(Effect, 0xd8);

// A side's effect manager (0x2dc bytes); the third side has one too. Only
// the fields known so far.
class EffectManager
{
public:
    // Frees the blocks of each effect and the effects. Returns 0. Clears
    // unknownCc where it should clear unknown04 (0x40d200).
    static int FreeEffects(EffectManager *mgr);
    unsigned char pad_0[0x18];
    ChainElem *calcElem;
    ChainElem *drawElem;
    unsigned char pad_20[0x30 - 0x20];
    Effect *effects;
    // The effects are unknown34 + unknown38 + 1.
    int unknown34;
    int unknown38;
};
