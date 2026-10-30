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
    // Zeroes the state, with no script running (0x415c90).
    void Clear();

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
    // Releases ANM slot 10 (0x415c50).
    static void ReleaseAnm();
    // Has the dialogue unload its music if the game frees its resources.
    // Returns 0 (0x418270).
    static int FreeResources(Gui *gui);
    // Frees the resources, removes the chain elements and deletes the
    // front, if any (0x4182a0).
    static void RemoveChain(Gui *gui);
    // Resets the front for a new round (0x4181e0).
    void Reset();
    // Returns unknown11ea8 (0x40d4f0).
    int GetUnknown11ea8();
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
    unsigned char pad_10958[0x1095c - 0x10958];
    // The player is not drawn while unknown1095c is 2 or more or
    // unknown10960 is over 30, unless its unknownA8 is set.
    int unknown1095c;
    int unknown10960;
    unsigned char pad_10964[0x10968 - 0x10964];
    Sprite unknown10968[8];
    unsigned char pad_11e88[0x11ea8 - 0x11e88];
    // Nonzero stops the player's combo from counting hits.
    int unknown11ea8;
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
    // Sets unknownAbcc and restarts timerAbd0 (0x415d50).
    void StartUnknownAbcc();
    // Starts script 0x43 on unknown59c8 (0x41a210).
    void StartUnknown59c8();
    // Starts scripts 0x44-0x47 on unknown5c6c and sets unknownA65c
    // (0x41a270).
    void StartUnknown5c6c();
    // Interrupts unknown5c6c with 1 and clears unknownA65c (0x41a2d0).
    void StopUnknown5c6c();
    // Starts scripts 0x48-0x4a on unknown66fc, the last showing digit
    // `digit` (sprite 0x35 on) (0x41a320).
    void StartUnknown66fc(int digit);
    // Starts script 0x4b on unknown6ee8[0] and 0x4c + `idx` on
    // unknown6ee8[1] (0x41a380).
    void StartUnknown6ee8(int idx);
    // Starts script 0x4e on unknown7430 (0x41a3c0).
    void StartUnknown7430();
    // Starts scripts 0x4f-0x54 on unknown76d4 and shows `frames` on them
    // with SetUnknown76d4Time (0x41a3e0).
    void StartUnknown76d4(int frames);
    // Shows `frames` as a time on unknown76d4's digits, clearing them
    // when it is negative (0x418950).
    void SetUnknown76d4Time(int frames);
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
    // Set while unknown5c6c shows, by StartUnknown5c6c.
    int unknownA65c;
    ChainElem *calcElem;
    ChainElem *drawElem;
    // The side's ANM file.
    AnmLoaded *anm;
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
