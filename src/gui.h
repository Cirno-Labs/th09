// The front: the interface drawn over the game ("FrontInf"), as th08's
// Gui.

#pragma once

#include "chain.h"
#include "sprite.h"
#include "timer.h"

// The dialogue state (0x1d70 bytes). Only the fields known so far.
struct GuiMsg
{
    // Unloads the music the scripts from `firstScript` on, ten of them,
    // play with opcode 7 (0x417520).
    void UnloadBgm(int firstScript);

    void *msgFile;
    // The instruction being run.
    void *instr;
    // The script running, -1 for none.
    int scriptIndex;
    unsigned char pad_c[0x14 - 0xc];
    Sprite portraits[8];
    Sprite unknown1534[2];
    Sprite unknown1a7c;
    unsigned char pad_1d20[0x1d40 - 0x1d20];
    Timer unknown1d40;
    unsigned char pad_1d4c[0x1d58 - 0x1d4c];
    Timer unknown1d58;
    unsigned char pad_1d64[0x1d70 - 0x1d64];
};
ASSERT_SIZE(GuiMsg, 0x1d70);

// Something of 0x14 bytes with an empty constructor; the linker folded
// it with D3DXVECTOR3's (0x4343d0).
struct GuiUnknownE86c
{
    GuiUnknownE86c()
    {
    }

    unsigned char pad_0[0x14];
};

// The front (0x1293c bytes), at GameManager::unknownA8. Only the fields
// known so far.
class Gui
{
public:
    // Zeroes the front. Inline; the copy is at 0x415d40.
    void Init()
    {
        memset(this, 0, sizeof(Gui));
    }
    // The update chain callback and its added callback, and the draw
    // chain callback (0x417630, 0x418480, 0x417c10).
    static int OnUpdate(Gui *gui);
    static int AddedCallback(Gui *gui);
    static int OnDraw(Gui *gui);
    // Creates the front, zeroed, and adds its update element (priority
    // 0x16) and draw element (0x1c). Returns it, or NULL if adding fails
    // (0x418840).
    static Gui *RegisterChain();
    // Preloads front.anm into ANM slot 10; returns 1 if that fails
    // (0x415c30).
    static int LoadAnm();
    // Has the dialogue unload its music if the game frees its resources.
    // Returns 0 (0x418270).
    static int FreeResources(Gui *gui);
    // Frees the resources, removes the chain elements and deletes the
    // front, if any (0x4182a0).
    static void RemoveChain(Gui *gui);
    // Resets the front for a new round (0x4181e0).
    void Reset();
    // Whether a dialogue script runs, or is about to (-2), as th08's.
    // Inline; the copy is at 0x404980.
    int IsMsgRunning()
    {
        return this->msg.scriptIndex >= 0 || this->msg.scriptIndex == -2 ? 1 : 0;
    }

    ChainElem *calcElem;
    ChainElem *drawElem;
    // SpellCard::Reset copies it.
    int unknown08;
    Sprite unknownC[0x58];
    GuiUnknownE86c unknownE86c[10];
    unsigned char pad_e934[0xe944 - 0xe934];
    GuiMsg msg;
    Sprite unknown106b4;
    unsigned char pad_10958[0x10968 - 0x10958];
    Sprite unknown10968[8];
    unsigned char pad_11e88[0x11eac - 0x11e88];
    Sprite unknown11eac[2];
    Sprite unknown123f4[2];
};
ASSERT_SIZE(Gui, 0x1293c);

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
