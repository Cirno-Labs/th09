// The extra attacks the sides send each other ("ExAttackCtrlInf").

#pragma once

#include "chain.h"

struct ExAttack;
// An extra attack's callbacks, which take the attack.
typedef int (*ExAttackCallback)(ExAttack *attack);

// An extra attack (0x4c bytes). Only the fields known so far.
struct ExAttack
{
    unsigned char pad_0[0xc];
    // Whether the attack runs.
    int active;
    unsigned char pad_10[0x1c - 0x10];
    void *unknown1c;
    unsigned char pad_20[0x48 - 0x20];
    // Called when the attack is freed, if set.
    ExAttackCallback onFree;
};
ASSERT_SIZE(ExAttack, 0x4c);

// The extra attacks (0x4c80 bytes), at GameManager::unknownAc. Only the
// fields known so far.
class ExAttackManager
{
public:
    // Frees the attacks, removes the manager's chain elements and deletes
    // it, if any (0x4156a0).
    static void RemoveChain(ExAttackManager *mgr);

    ChainElem *calcElem;
    ChainElem *drawElem;
    ChainElem *drawElem2;
    unsigned char pad_c[0x1c - 0xc];
    ExAttack attacks[0x100];
    unsigned char pad_4c1c[0x4c80 - 0x4c1c];
};
ASSERT_SIZE(ExAttackManager, 0x4c80);
