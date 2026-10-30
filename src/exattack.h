// The extra attacks the sides send each other ("ExAttackCtrlInf").

#pragma once

#include "chain.h"
#include "timer.h"

struct ExAttack;
// An extra attack's callbacks, which take the attack.
typedef int (*ExAttackCallback)(ExAttack *attack);

// An extra attack (0x4c bytes). Only the fields known so far.
struct ExAttack
{
    unsigned char pad_0[0xc];
    // Whether the attack runs.
    int active;
    Timer unknown10;
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
    // Ends and clears the attacks for a new round (0x4156f0).
    void Reset();
    // The update and draw chain callbacks (0x415340, 0x415460,
    // 0x415580).
    static int OnUpdate(ExAttackManager *mgr);
    static int OnDraw(ExAttackManager *mgr);
    static int OnDraw2(ExAttackManager *mgr);
    // The update element's added callback: sets unknown14 and unknown18
    // by each side's character and the difficulty, then resets the
    // attacks. Returns 0 (0x415750).
    static int AddedCallback(ExAttackManager *mgr);
    // Creates the manager, zeroed, and adds its update element (priority
    // 0xe) and draw elements (0x10 and 0x1d). Returns it, or NULL if
    // adding fails (0x4157d0).
    static ExAttackManager *RegisterChain();

    ChainElem *calcElem;
    ChainElem *drawElem;
    ChainElem *drawElem2;
    unsigned char pad_c[0x14 - 0xc];
    int unknown14;
    int unknown18;
    // The loops stop before the last.
    ExAttack attacks[0x101];
    unsigned char pad_4c68[0x4c80 - 0x4c68];
};
ASSERT_SIZE(ExAttackManager, 0x4c80);

// Two numbers by character, which ExAttackManager::AddedCallback adds
// the first of to the second times the difficulty (0x4a1128, 0x4a117c).
extern int g_Unknown4a1128[];
extern int g_Unknown4a117c[];
