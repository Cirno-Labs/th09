// The enemies, one manager per side ("EnemyCtrlInf").

#pragma once

#include "chain.h"

// An ECL file and the state of its scripts (0x188 bytes). Only the
// fields known so far.
struct EclManager
{
    // Loads the ECL file `path`. Returns 0, or -1 on failure (0x405660).
    int Load(char *path);
    // Frees the ECL file (0x406830).
    void Unload();

    void *eclFile;
    unsigned char pad_4[0x188 - 0x4];
};
ASSERT_SIZE(EclManager, 0x188);

// An enemy (0x5430 bytes). Only the fields known so far.
struct Enemy
{
    // Frees the blocks of unknown33d8 (0x4100e0).
    void FreeUnknown33d8();

    unsigned char pad_0[0x33d8];
    void *unknown33d8[4];
    unsigned char pad_33e8[0x5430 - 0x33e8];
};
ASSERT_SIZE(Enemy, 0x5430);

// A side's enemy manager (0x2ac450 bytes). Only the fields known so far.
class EnemyManager
{
public:
    // Frees the enemies' resources, removes the manager's chain elements
    // and deletes it, if any (0x411f80).
    static void RemoveChain(EnemyManager *mgr);
    // Resets the enemies for a new round (0x411fd0).
    void Reset();

    // enemy.ecl, and the character's plXX.ecl.
    EclManager ecl;
    EclManager playerEcl;
    ChainElem *calcElem;
    ChainElem *drawElem;
    ChainElem *drawElem2;
    unsigned char pad_31c[0x5758 - 0x31c];
    Enemy enemies[0x80];
};
