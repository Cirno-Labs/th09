// The spell cards, one per side ("CardAttackInf"), as th08's SpellCard.

#pragma once

#include "chain.h"

class SpellCard;
// A spell card's callbacks, which take the spell card.
typedef int (*SpellCardCallback)(SpellCard *card);

// A side's spell card (0x1368 bytes). Only the fields known so far.
class SpellCard
{
public:
    // Ends the spell card, removes its chain elements and deletes it, if
    // any (0x404570).
    static void RemoveChain(SpellCard *card);

    unsigned char pad_0[0xc];
    ChainElem *calcElem;
    ChainElem *drawElem;
    unsigned char pad_14[0x1330 - 0x14];
    SpellCardCallback unknown1330;
    unsigned char pad_1334[0x133c - 0x1334];
    SpellCardCallback unknown133c;
};
