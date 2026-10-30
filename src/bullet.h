// The bullets, one manager per side.

#pragma once

#include "chain.h"

// A side's bullet manager, which loads etama.anm. Only the fields known
// so far.
class BulletManager
{
public:
    // Removes the manager's chain elements and deletes it, if any
    // (0x414180).
    static void RemoveChain(BulletManager *mgr);

    unsigned char pad_0[0x25e184];
    ChainElem *calcElem;
    ChainElem *drawElem;
};
