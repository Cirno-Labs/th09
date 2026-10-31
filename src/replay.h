// The replays: recording and playing back ("ReplayInf").

#pragma once

#include "chain.h"
#include "unsorted.h"

// Part of a replay's header, with a copy of the config. Only what is
// known so far.
struct ReplayInfo
{
    // Constructs the config (0x420830).
    ReplayInfo();

    unsigned char pad_0[0x1c];
    GameConfig config;
};

// A replay's header (0x1ec bytes): ReplayManager allocates one, and the
// title menu keeps one for each replay file. Only what is known so far.
struct ReplayHeader
{
    // Constructs info (0x420c40).
    ReplayHeader();

    // "T9RP", version 2, as th08's header.
    char magic[4];
    short version;
    // Must be 0 for the replay to load.
    BYTE unknown6;
    unsigned char pad_7[0xc - 0x7];
    // The size of the file's header and compressed data.
    int size;
    // The sum of the bytes from key on, plus 0x3f000318.
    int checksum;
    unsigned char pad_14[0x15 - 0x14];
    // What the bytes from compressedSize on are encrypted with, rising by
    // 7 a byte.
    BYTE key;
    unsigned char pad_16[0x18 - 0x16];
    int compressedSize;
    // The size of the data once decompressed.
    int dataSize;
    // Offsets in the replay, made pointers once it is loaded.
    void *unknown20[10];
    void *unknown48[10];
    void *unknown70[10];
    void *unknown98[10];
    ReplayInfo info;
    unsigned char pad_1a8[0x1d4 - 0x1a8];
    // The executable the replay was recorded with, for
    // App::CheckVersionFile.
    DWORD exeSize;
    DWORD exeChecksum;
    char exeVersion[0x10];
};
ASSERT_SIZE(ReplayHeader, 0x1ec);

// Decrypts and checks the replay file `data`, `size` bytes, and returns
// it decompressed, as th08's ValidateReplay, with what follows the
// compressed data copied after it and the offsets made pointers. Frees
// `data`. Returns NULL if the file is not a valid replay (0x4205e0).
ReplayHeader *ValidateReplay(void *data, int size);

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
    // Runs the added callback of `mode`, 0 or 1, on `mgr` (0x4217e0).
    static void RunAddedCallback(ReplayManager *mgr, int mode);
    // Removes the manager's chain elements, frees its blocks and deletes
    // it, if any (0x420c10).
    static void RemoveChain(ReplayManager *mgr);
    // Frees the manager's blocks and resources, removes its update
    // element and deletes it, if any (0x420ad0).
    static void Release(ReplayManager *mgr);

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
