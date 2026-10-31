// The spell cards, one per side ("CardAttackInf"), as th08's SpellCard.

#pragma once

#include "chain.h"
#include "sprite.h"
#include "timer.h"

struct GameSide;

class SpellCard;
// A spell card's callbacks, which take the spell card.
typedef int (*SpellCardCallback)(SpellCard *card);

// A side's spell card (0x1368 bytes). Only the fields known so far.
class SpellCard
{
public:
    // The update and draw chain callbacks (0x4041f0, 0x404410).
    static int OnUpdate(SpellCard *card);
    static int OnDraw(SpellCard *card);
    // The update element's added callback: resets the spell card.
    // Returns 0 (0x404560).
    static int AddedCallback(SpellCard *card);
    // Ends the spell card, removes its chain elements and deletes it, if
    // any (0x404570).
    static void RemoveChain(SpellCard *card);
    // Resets the spell card for a new round (0x403d80).
    void Reset();
    // Creates side `sideIdx`'s spell card, zeroed, and adds its update
    // element (priority 0x11 + `sideIdx`) and draw element (0x18 +
    // `sideIdx`). Returns it, or NULL if adding fails (0x404600).
    static SpellCard *RegisterChain(int sideIdx);

    int sideIdx;
    GameSide *side;
    GameSide *otherSide;
    ChainElem *calcElem;
    ChainElem *drawElem;
    // Gui::anm, as Reset copies it.
    AnmLoaded *anm;
    unsigned char pad_18[0x98 - 0x18];
    Timer unknown98;
    unsigned char pad_a4[0xac - 0xa4];
    Sprite unknownAc[2];
    Sprite unknown5f4[5];
    unsigned char pad_1328[0x1330 - 0x1328];
    SpellCardCallback unknown1330;
    unsigned char pad_1334[0x133c - 0x1334];
    SpellCardCallback unknown133c;
    unsigned char pad_1340[0x1360 - 0x1340];
    // Blocks some types' callbacks allocate and free.
    void *unknown1360;
    void *unknown1364;
};
ASSERT_SIZE(SpellCard, 0x1368);
