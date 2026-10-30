// The front: the interface drawn over the game ("FrontInf"), as th08's
// Gui.

#pragma once

#include "chain.h"
#include "sprite.h"
#include "timer.h"

// The dialogue state. Only the fields known so far.
struct GuiMsg
{
    // Unloads the music the scripts from `firstScript` on, ten of them,
    // play with opcode 7 (0x417520).
    void UnloadBgm(int firstScript);

    void *msgFile;
};

// The front (0x1293c bytes), at GameManager::unknownA8. Only the fields
// known so far.
class Gui
{
public:
    // Has the dialogue unload its music if the game frees its resources.
    // Returns 0 (0x418270).
    static int FreeResources(Gui *gui);
    // Frees the resources, removes the chain elements and deletes the
    // front, if any (0x4182a0).
    static void RemoveChain(Gui *gui);
    // Resets the front for a new round (0x4181e0).
    void Reset();

    ChainElem *calcElem;
    ChainElem *drawElem;
    // SpellCard::Reset copies it.
    int unknown08;
    unsigned char pad_c[0xe944 - 0xc];
    GuiMsg msg;
};

struct GameSide;

// A part of GuiSide with two sprites (0x554 bytes). Only the fields
// known so far.
struct GuiSideA678
{
    int unknown0;
    // GuiSide's resources include ANM slot 7 while it is set.
    int unknown4;
    int unknown8;
    Sprite sprites[2];
};
ASSERT_SIZE(GuiSideA678, 0x554);

// A side's part of the front ("FrontSideInf", 0xabe8 bytes). Only the
// fields known so far.
class GuiSide
{
public:
    // Zeroes the side's front. Inline; the copy is at 0x418920.
    void Init()
    {
        memset(this, 0, sizeof(GuiSide));
    }
    // The update chain callback and its added callback, and the draw
    // chain callback (0x418a90, 0x41a000, 0x4193e0).
    static int OnUpdate(GuiSide *side);
    static int AddedCallback(GuiSide *side);
    static int OnDraw(GuiSide *side);
    // Removes the chain elements and deletes the side's front, if any,
    // after unloading ANM slot 7 if unknownA678.unknown4 is set
    // (0x41a1d0).
    static void RemoveChain(GuiSide *side);
    // Resets the side's front for a new round (0x41a230).
    void Reset();
    // Creates side `sideIdx`'s front, zeroed, and adds its update element
    // (priority 0x17 + `sideIdx`) and draw element (0x1a + `sideIdx`).
    // Returns it, or NULL if adding fails (0x41a5d0).
    static GuiSide *RegisterChain(int sideIdx);

    Sprite unknown0[0xb];
    Sprite unknown1d0c[5];
    Sprite unknown2a40[7];
    Sprite unknown3cbc;
    Sprite unknown3f60[10];
    Sprite unknown59c8;
    Sprite unknown5c6c[4];
    Sprite unknown66fc[3];
    Sprite unknown6ee8[2];
    Sprite unknown7430;
    Sprite unknown76d4[6];
    Sprite unknown86ac[7];
    Sprite unknown9928[5];
    unsigned char pad_a65c[0xa660 - 0xa65c];
    ChainElem *calcElem;
    ChainElem *drawElem;
    unsigned char pad_a668[0xa66c - 0xa668];
    int sideIdx;
    // The side, and the other one.
    GameSide *side;
    GameSide *otherSide;
    GuiSideA678 unknownA678;
    // Reset turns it from set to 2, restarting timerAbd0 at 30.
    int unknownAbcc;
    Timer timerAbd0;
    Timer timerAbdc;
};
ASSERT_SIZE(GuiSide, 0xabe8);
