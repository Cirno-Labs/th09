// The replays: recording and playing back ("ReplayInf").

#pragma once

#include "chain.h"

// The replay manager, at GameManager::unknownD4. Only the fields known so
// far.
class ReplayManager
{
public:
    // Empty; the linker folded it with D3DXVECTOR3's (0x4343d0).
    ReplayManager()
    {
    }

    // The update callbacks and added callbacks of mode 0, which also
    // runs OnUpdateMode0b once from RegisterChain (0x4202a0, 0x421140,
    // 0x420190), and of mode 1, which takes a path (0x4204b0, 0x420840,
    // 0x420470).
    static int OnUpdateMode0(ReplayManager *mgr);
    static int AddedCallbackMode0(ReplayManager *mgr);
    static int OnUpdateMode0b(ReplayManager *mgr);
    static int OnUpdateMode1(ReplayManager *mgr);
    static int AddedCallbackMode1(ReplayManager *mgr);
    static int OnUpdateMode1b(ReplayManager *mgr);
    // Creates the replay manager, zeroed, in `mode`, and adds its update
    // elements: in mode 0, calcElem (priority 0x1a) and unknownElem15c
    // (7); in mode 1, which keeps `path`, calcElem (6) and
    // unknownElem158 (0x1b). Returns it, or NULL if adding calcElem
    // fails (0x421670).
    static ReplayManager *RegisterChain(int mode, char *path);
    // Removes the manager's chain elements, frees its blocks and deletes
    // it, if any (0x420c10).
    static void RemoveChain(ReplayManager *mgr);

    // OnUpdateMode1b returns 6 while a dialogue runs unless it is 2
    // modulo 3.
    int unknown0;
    unsigned char pad_4[0x8 - 0x4];
    void *unknown08;
    unsigned char pad_c[0x110 - 0xc];
    void *unknown110;
    int mode;
    char *path;
    unsigned char pad_11c[0x150 - 0x11c];
    ChainElem *calcElem;
    unsigned char pad_154[0x158 - 0x154];
    ChainElem *unknownElem158;
    ChainElem *unknownElem15c;
    unsigned char pad_160[0x164 - 0x160];
};
ASSERT_SIZE(ReplayManager, 0x164);
