// The player, one per side ("PlayerInf").

#pragma once

#include "chain.h"
#include "gamemanager.h"

// A side's player (0x30f70 bytes). Only the fields known so far.
class Player
{
public:
    // Frees the player's resources, removes its chain elements and
    // deletes it, if any (0x41ced0).
    static void RemoveChain(Player *player);
    // Resets the player for a new round (0x41ebc0).
    void Reset();

    unsigned char pad_0[0x8];
    // The side, 0 or 1.
    int sideIdx;
    GameSide *side;
    unsigned char pad_10[0x14 - 0x10];
    ChainElem *calcElem;
    ChainElem *drawElem;
    ChainElem *drawElem2;
    unsigned char pad_20[0x1b88 - 0x20];
    D3DXVECTOR3 pos;
    unsigned char pad_1b94[0x30338 - 0x1b94];
    // The character's shot file (plXX.sht).
    void *shtFile;
};

// Each side's shot file, kept while the side keeps its character
// (0x4a8160).
extern void *g_ShtFiles[2];
