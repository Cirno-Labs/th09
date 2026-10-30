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

    unsigned char pad_0[0x8];
    // The side, 0 or 1.
    int sideIdx;
    GameSide *side;
    unsigned char pad_10[0x14 - 0x10];
    ChainElem *calcElem;
    ChainElem *drawElem;
    ChainElem *drawElem2;
    unsigned char pad_20[0x30338 - 0x20];
    // The character's shot file (plXX.sht).
    void *shtFile;
};

// Each side's shot file, kept while the side keeps its character
// (0x4a8160).
extern void *g_ShtFiles[2];
