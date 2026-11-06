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

    BYTE randomByte;
    // 'a' and 0x150: the version, 1.50a.
    char versionLetter;
    WORD version;
    // g_Hscr's date, and the player's name, when it is saved.
    char date[0xa];
    char name[9];
    BYTE difficulty;
    unsigned char pad_18[0x1c - 0x18];
    GameConfig config;
};

// The 0x20 byte block a replay keeps of each side (and the third input
// set) of a round, before its inputs.
struct ReplayRound
{
    // Goes to the side's GameStats score and guiScore.
    DWORD score;
    // The round's Rng seed (side 0's).
    WORD rngSeed;
    BYTE character;
    // GameSide::unknown28, unknown2c; GameStats::lives.
    BYTE unknown7;
    BYTE lives;
    // GameManager::unknown138 (side 0's).
    char unknown9;
    WORD unknownA;
    // GameManager::stage (side 0's).
    BYTE stage;
    unsigned char pad_d[0x10 - 0xd];
    // GameStats::extendsGiven (side 0's).
    int extendsGiven;
    unsigned char pad_14[0x20 - 0x14];
};
ASSERT_SIZE(ReplayRound, 0x20);

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
    // Set when it is saved.
    BYTE isSaved;
    unsigned char pad_8[0xc - 0x8];
    // The size of the file's header and compressed data.
    int size;
    // The sum of the bytes from key on, plus 0x3f000318.
    int checksum;
    // Random; never read.
    BYTE randomFill;
    // What the bytes from compressedSize on are encrypted with, rising by
    // 7 a byte.
    BYTE key;
    unsigned char pad_16[0x18 - 0x16];
    int compressedSize;
    // The size of the data once decompressed.
    int dataSize;
    // Offsets in the replay, made pointers once it is loaded: by stage,
    // three 0x20 byte blocks each followed by the inputs of a ReplayChunk
    // list, and the bytes kept every 30 frames.
    void *unknown20[3][10];
    void *unknown98[10];
    ReplayInfo info;
    unsigned char pad_1a8[0x1d0 - 0x1a8];
    // 30.
    int replayFormatVersion;
    // The executable the replay was recorded with, for
    // App::CheckVersionFile.
    DWORD exeSize;
    DWORD exeChecksum;
    char exeVersion[8];
    // GameManager::mode and unknown114.
    BYTE mode;
    BYTE unknown1e5;
    // GameManager::initialLives, and each side's unknown30.
    BYTE unknown1e6[2];
    BYTE unknown1e8[2];
    unsigned char pad_1ea[0x1ec - 0x1ea];
};
ASSERT_SIZE(ReplayHeader, 0x1ec);

// Decrypts and checks the replay file `data`, `size` bytes, and returns
// it decompressed, as th08's ValidateReplay, with what follows the
// compressed data copied after it and the offsets made pointers. Frees
// `data`. Returns NULL if the file is not a valid replay (0x4205e0).
ReplayHeader *ValidateReplay(void *data, int size);

// The text section after a replay's data, as th08's: "USER", its size
// counted from `magic`, and a type.
struct ReplayUserHeader
{
    DWORD magic;
    int size;
    BYTE type;
};

// A block of a stage's recording: three input streams of `count` WORDs,
// a byte every 30 frames, and the next block.
struct ReplayChunk
{
    WORD *inputs[3];
    BYTE *extra;
    int count;
    ReplayChunk *next;
};
ASSERT_SIZE(ReplayChunk, 0x18);

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
    // Saves the recording to `path` with the player's `name`, as th08's
    // SaveReplay, with a "USER" section of the game's details after the
    // data; does nothing if either is NULL. Returns 0 (0x420c60).
    static int SaveReplay(ReplayManager *mgr, char *path, char *name);

    // OnUpdateMode1b returns 6 while a dialogue runs unless it is 2
    // modulo 3.
    int unknown0;
    // How many frames OnUpdateMode0 recorded while bit 9 of stateFlags
    // was set; it stops at 3.
    int unknown4;
    ReplayHeader *unknown08;
    // Where each player's recorded inputs and the recorded frame rates
    // are read from, as th08's cursor and extraCursor.
    WORD *inputCursors[3];
    BYTE *extraCursor;
    // The recording of each stage.
    ReplayChunk chunks[10];
    // The chunk being recorded, as th08's.
    ReplayChunk *curChunk;
    void *unknown110;
    int mode;
    char *path;
    unsigned char pad_11c[0x150 - 0x11c];
    ChainElem *calcElem;
    unsigned char pad_154[0x158 - 0x154];
    ChainElem *unknownElem158;
    ChainElem *unknownElem15c;
    // The RNG seed the recording starts from, and flags; bit 8 is
    // GameManager::unknown130 at the start.
    WORD seed;
    WORD unknown162;
};
ASSERT_SIZE(ReplayManager, 0x164);
