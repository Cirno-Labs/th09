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
    // Draws background layer `layer` (0x4026c0).
    void DrawLayer(int layer);

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
    // Drawn under the layers while spellCardState is at most 1; the
    // first is placed by OnDraw.
    Sprite backgroundLayer0;
    Sprite backgroundLayer1;
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
    unsigned char pad_c6c[0xc7c - 0xc6c];
    // OnDraw clears to the fog color if this is opaque, draws the fog
    // color over the arena if it is otherwise nonzero.
    D3DCOLOR clearColor;
    unsigned char pad_c80[0xc84 - 0xc80];
    Timer unknownC84;
    unsigned char pad_c90[0xc94 - 0xc90];
    float fogNear;
    float fogFar;
    D3DCOLOR fogColor;
    unsigned char pad_ca0[0xcbc - 0xca0];
    Timer unknownCbc;
    unsigned char pad_cc8[0xccc - 0xcc8];
    // Cleared by Reset. The layers are drawn while it is at most 1.
    int spellCardState;
    unsigned char pad_cd0[0xcd4 - 0xcd0];
    // OnDraw clears the target to black once if set.
    int clearScreenOnce;
    unsigned char pad_cd8[0xce0 - 0xcd8];
    Sprite unknownCe0[0x20];
    Sprite unknown6160;
    unsigned char pad_6404[0x642c - 0x6404];
    // OnDraw modulates sprites by it if its alpha is nonzero.
    union
    {
        D3DCOLOR color;
        struct
        {
            BYTE b;
            BYTE g;
            BYTE r;
            BYTE a;
        };
    } tintColor;
    // OnDraw stops modulating sprites unless set.
    int keepColorModulation;
    unsigned char pad_6434[0x6444 - 0x6434];
    D3DXVECTOR3 unknown6444[0x20];
};
ASSERT_SIZE(Stage, 0x65c4);
