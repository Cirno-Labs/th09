// The front: the interface drawn over the game ("FrontInf"), as th08's
// Gui.

#pragma once

#include "chain.h"

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
    unsigned char pad_8[0xe944 - 0x8];
    GuiMsg msg;
};

// A side's part of the front ("FrontSideInf", 0xabe8 bytes). Only the
// fields known so far.
class GuiSide
{
public:
    // Removes the chain elements and deletes the side's front, if any,
    // after unloading ANM slot 7 if unknownA67c is set (0x41a1d0).
    static void RemoveChain(GuiSide *side);
    // Resets the side's front for a new round (0x41a230).
    void Reset();

    unsigned char pad_0[0xa660];
    ChainElem *calcElem;
    ChainElem *drawElem;
    unsigned char pad_a668[0xa67c - 0xa668];
    int unknownA67c;
};
