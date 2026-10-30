// The stage: its background and scrolling, one per side.

#pragma once

#include "chain.h"

// A side's stage. Only the fields known so far.
class Stage
{
public:
    // Removes the stage's chain elements and deletes it, if any
    // (0x402530).
    static void CutChain(Stage *stage);

    ChainElem *calcElem;
    ChainElem *drawElem;
    // A second draw element.
    ChainElem *drawElem2;
};
