// The stage: its background and scrolling, one per side.

#pragma once

#include "chain.h"
#include "sprite.h"
#include "timer.h"

struct GameSide;

// A side's stage ("BGScrollInf", 0x65c4 bytes). Only the fields known so
// far.
class Stage
{
public:
    // Zeroes the stage (0x403a40).
    Stage();

    // Removes the stage's chain elements and deletes it, if any
    // (0x402530).
    static void RemoveChain(Stage *stage);
    // Resets the stage for a new round (0x4011c0).
    void Reset();
    // The update chain callback and its added and deleted callbacks
    // (0x402350, 0x403830, 0x4024d0). DeletedCallback frees the stage's
    // ANM, slot 4, if resources are freed, and its two buffers.
    static int OnUpdate(Stage *stage);
    static int AddedCallback(Stage *stage);
    static int DeletedCallback(Stage *stage);
    // The two draw chain callbacks (0x4033e0, 0x403680).
    static int OnDraw(Stage *stage);
    static int OnDraw2(Stage *stage);
    // Creates side `sideIdx`'s stage and adds its update element
    // (priority 8 + `sideIdx`) and draw elements (6 + `sideIdx` and 8 +
    // `sideIdx`). Returns it, or NULL if adding fails (0x403ae0).
    static Stage *RegisterChain(int sideIdx);

    ChainElem *calcElem;
    ChainElem *drawElem;
    // A second draw element.
    ChainElem *drawElem2;
    int sideIdx;
    // The side, and the other one.
    GameSide *side;
    GameSide *otherSide;
    unsigned char pad_18[0x24 - 0x18];
    // Freed by DeletedCallback.
    void *unknown24;
    unsigned char pad_28[0x2c - 0x28];
    Sprite unknown2c;
    Sprite unknown2d0;
    Sprite unknown574;
    unsigned char pad_818[0x820 - 0x818];
    // Freed by DeletedCallback.
    void *unknown820;
    unsigned char pad_824[0xc08 - 0x824];
    Timer unknownC08[5];
    unsigned char pad_c44[0xc58 - 0xc44];
    Timer unknownC58;
    unsigned char pad_c64[0xc68 - 0xc64];
    // Cleared by RegisterChain.
    int unknownC68;
    unsigned char pad_c6c[0xc84 - 0xc6c];
    Timer unknownC84;
    unsigned char pad_c90[0xcbc - 0xc90];
    Timer unknownCbc;
    unsigned char pad_cc8[0xccc - 0xcc8];
    // Cleared by Reset.
    int unknownCcc;
    unsigned char pad_cd0[0xce0 - 0xcd0];
    Sprite unknownCe0[0x20];
    Sprite unknown6160;
    unsigned char pad_6404[0x6444 - 0x6404];
    D3DXVECTOR3 unknown6444[0x20];
};
ASSERT_SIZE(Stage, 0x65c4);
