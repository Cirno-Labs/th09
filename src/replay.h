// The replays: recording and playing back ("ReplayInf").

#pragma once

#include "chain.h"

// The replay manager, at GameManager::unknownD4. Only the fields known so
// far.
class ReplayManager
{
public:
    // Removes the manager's chain elements, frees its blocks and deletes
    // it, if any (0x420c10).
    static void RemoveChain(ReplayManager *mgr);

    unsigned char pad_0[0x8];
    void *unknown08;
    unsigned char pad_c[0x110 - 0xc];
    void *unknown110;
    unsigned char pad_114[0x150 - 0x114];
    ChainElem *calcElem;
    unsigned char pad_154[0x158 - 0x154];
    ChainElem *unknownElem158;
    ChainElem *unknownElem15c;
};
