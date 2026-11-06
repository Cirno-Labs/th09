// The front: the interface drawn over the game ("FrontInf"), as th08's
// Gui.

#pragma once

#include "chain.h"
#include "sprite.h"
#include "timer.h"

// The arguments of most dialogue opcodes, as th08's MsgArgs: a
// portrait, then a sprite, script or line number, then the text.
struct MsgArgs
{
    short who;
    union
    {
        short index;
        BYTE value;
    };
    char text[1];
};

// The arguments of opcodes 15 and 17, as th08's MsgPortraitArgs: the
// portrait that speaks, then a sprite for each side's portraits
// (negative to keep them).
struct MsgPortraitArgs
{
    int who;
    int sprites[2];
};

// An instruction of a dialogue script, as th08's MsgInstr: its
// arguments follow, argsSize bytes of them.
struct MsgInstr
{
    WORD time;
    BYTE opcode;
    BYTE argsSize;
    union
    {
        MsgArgs args;
        MsgPortraitArgs portraitArgs;
        int i;
        BYTE b;
    };
};

// A dialogue file: its scripts, after a count so it seems.
struct MsgFile
{
    int count;
    struct
    {
        MsgInstr *script;
        // GuiMsg::StartRandom's odds of the script, out of 0x100.
        int weight;
    } scripts[1];
};

// The dialogue files once loaded, for side 0 and side 1 (0x4a7d88,
// 0x4a7d8c); GuiMsg::UnloadBgm frees them.
extern MsgFile *g_MsgFile;
extern MsgFile *g_MsgFile2;

// The dialogue state (0x1d70 bytes). Only the fields known so far.
struct GuiMsg
{
    // Starts script `index` of side `side`'s file (msgFile2 for side 1),
    // as th08's StartMsgScript, with clear text sprites in the text
    // ANM file's scripts 0 and 1 (0x416160).
    void Start(int index, int side);
    // Starts, as Start, a script of side `side`'s file picked at random
    // by weight among the ten of the other side's character, or else
    // among scripts 210-219, script 210 if none is picked (0x4162d0).
    void StartRandom(int side);
    // Draws the portraits (2 and 3 mirrored, the others in an order by
    // speaker), the text box growing over its first 40 frames, and the
    // text, as th08's DrawMsg. Returns 0, or -1 with no script running
    // (0x417130).
    int Draw();
    // Runs the script up to the current frame and steps the sprites, as
    // th08's RunMsg with th09's opcodes. Returns 0, or -1 once no script
    // runs (0x416590).
    int Run();
    // Unloads the music the scripts from `firstScript` on, ten of them,
    // play with opcode 7 (in mode 2 GameManager::unk_f0's unk_c),
    // then frees the dialogue files and clears the state, if a file is
    // loaded (0x417520).
    void UnloadBgm(int firstScript);
    // Zeroes the state, with no script running (0x415c90).
    void Clear();
    // Unloads the music and the files as UnloadBgm, then loads the
    // dialogue file `path` (in mode 2 `path2` as side 1's) unless it is
    // loaded, turning its script offsets into pointers, and loads the
    // music UnloadBgm would unload. Returns 0, or -1 if the file does
    // not load (0x4182e0).
    int Load(char *path, char *path2, int firstScript);

    MsgFile *msgFile;
    // The instruction being run.
    MsgInstr *instr;
    // The script running, -1 for none.
    int scriptIndex;
    // Side 1's file.
    MsgFile *msgFile2;
    // The side whose script runs.
    int side;
    Sprite portraits[8];
    // The two lines of text.
    Sprite text[2];
    Sprite nameplate;
    // The speakers' text and shadow colors, as th08's.
    D3DCOLOR textColors[4];
    D3DCOLOR shadowColors[4];
    // The frame the script has reached, as th08's timer.
    Timer timer;
    // Frames waited on opcode 4, as th08's.
    int waitFrames;
    int minWaitFrames;
    int fontSize;
    // Started at 1 by opcode 24, the frames the text box has grown;
    // Draw shows the box once it is above 0.
    Timer boxTimer;
    // As th08's.
    BYTE speaker;
    BYTE newPage;
    BYTE nextLine;
    BYTE activePortrait;
    // Raised by opcode 6 and counted down by the interpreter (0x416590),
    // as th08's.
    DWORD timelineRelease;
    BYTE skippable;
    // Whether Draw shows the text box, as th08's.
    BYTE textBoxShown;
    unsigned char pad_1d6e[0x1d70 - 0x1d6e];
};
ASSERT_SIZE(GuiMsg, 0x1d70);

// A vertex with a transformed position and a color, as
// VertexDiffuseXyzrwh, with an empty constructor; the linker folded it
// with D3DXVECTOR3's (0x4343d0).
struct GuiUnknownE86c
{
    GuiUnknownE86c()
    {
    }

    D3DXVECTOR3 pos;
    float w;
    D3DCOLOR diffuse;
};

// The front (0x1293c bytes), at GameManager::unk_a8. Only the fields
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
    // chain callback (0x417630, 0x418480, 0x417c10). The update callback
    // steps the sprites, shows the piece of music playing, flashes the
    // line strips while unk_e93c counts down and runs the dialogue.
    // At the end of a round (unk_1095c) it counts unk_10960: at 0
    // it ends the combos and clears the play areas, at 30 it tints and
    // shakes the loser's side, at 60 it takes a life from side 0 or
    // starts the winner's dialogue, at 160 and 190 (once unk_11ea4
    // is set) it starts the sides' unk_abcc and resets the round.
    // From unk_1095c 2, once the dialogue ends, it moves on to the
    // next stage, the ending or the continue menu. Returns 1. The draw
    // callback
    // draws the frame sprites, the enemy markers, the two line strips
    // and the dialogue, then while unk_1095c is set the round end
    // sprites of unk_10968 (placed from their posOffset in the play
    // area) and, from unk_1095c 2 and past unk_10960 20, the
    // bonuses on the winner's side (unk_10964), and last the clock
    // (with the stage outside mode 2). Returns 1.
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
    // Counts a round for side `side`, a win so it seems; once it has
    // unk_104 of them, starts the win and lose sprites. Sets
    // unk_1095c to 1 and unk_11ea8 (0x415d70).
    void CountRoundWin(int side);
    // Places side `side`'s marker of the enemy at `pos` under its play
    // area, more transparent the nearer the player is, or hides it while
    // pos is out of the play area (0x416080).
    void UpdateEnemyMarker(int side, D3DXVECTOR3 *pos);
    // Ends the round for the winner, side unk_10964: starts sprites
    // 0x3f and 0x42, sets unk_1095c to 2, works out the bonuses from
    // its player (and in unk_fc 8 its lives) and adds their total to
    // its score; in modes 0 and 1, when side 0 won, opens the record's
    // entry for the other's character (0x415e50).
    void ScoreRoundBonus();
    // Resets the front for a new round (0x4181e0).
    void Reset();
    // Returns unk_11ea8 (0x40d4f0).
    int GetUnknown11ea8()
    {
        return this->unk_11ea8;
    }
    // Whether a dialogue script runs, or is about to (-2), as th08's.
    // Inline; the copy is at 0x404980.
    int IsMsgRunning()
    {
        return this->msg.scriptIndex >= 0 || this->msg.scriptIndex == -2 ? 1 : 0;
    }

    ChainElem *calcElem;
    ChainElem *drawElem;
    // The front's ANM file; SpellCard::Reset copies it.
    AnmLoaded *anm;
    Sprite unk_c[0x58];
    // Two line strips of five vertices, the outlines of the play areas.
    GuiUnknownE86c unk_e86c[2][5];
    // Each side's color and frames of something, set by Player::Die.
    D3DCOLOR unk_e934[2];
    int unk_e93c[2];
    GuiMsg msg;
    Sprite unk_106b4;
    // The g_BgmUnknown08 unk_106b4 last showed.
    int unk_10958;
    // The player is not drawn while unk_1095c is 2 or more or
    // unk_10960 is over 30, unless its unk_a8 is set.
    int unk_1095c;
    int unk_10960;
    int unk_10964;
    Sprite unk_10968[8];
    // The round's bonuses, from the winner's player and stats, and their
    // total, set by ScoreRoundBonus.
    int unk_11e88;
    int unk_11e8c;
    int unk_11e90;
    int unk_11e94;
    int unk_11e98;
    int unk_11e9c;
    int bonusTotal;
    int unk_11ea4;
    // Nonzero stops the player's combo from counting hits.
    int unk_11ea8;
    Sprite unk_11eac[2];
    // Each side's marker of the enemy's position.
    Sprite enemyMarkers[2];
};
ASSERT_SIZE(Gui, 0x1293c);

struct GameSide;

// pl06_fc_s.anm and pl06b_fc_s.anm, by GameSide::unk_30.
extern char *g_Pl06FcAnmFiles[2];

// A part of GuiSide with two sprites (0x554 bytes). Only the fields
// known so far.
struct GuiSideA678
{
    // SpellCard::Start runs scripts of it on the sprites.
    AnmLoaded *anm;
    // pl06_fc_s.anm or pl06b_fc_s.anm in ANM slot 7, loaded on side 0
    // when unk_fc is 8 and the side plays character 6; GuiSide's
    // resources include the slot while it is set.
    AnmLoaded *unk_4;
    int unk_8;
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
    // chain callback (0x418a90, 0x41a000, 0x4193e0). The update callback
    // steps the sprites and shows the player's combo, bonus, score,
    // lives, unk_a0, unk_a4, unk_30384 and unk_30388 and
    // the side's wins on them. The draw callback clears the side's
    // camera shake, places the sprites in its play area (some by the
    // player) and draws them; while unk_abcc is set it also sweeps
    // four black triangles over the play area by timerAbd0 (in for 1,
    // out for 2). Both return 1.
    static int OnUpdate(GuiSide *side);
    static int AddedCallback(GuiSide *side);
    static int OnDraw(GuiSide *side);
    // Removes the chain elements and deletes the side's front, if any,
    // after unloading ANM slot 7 if unk_a678.unk_4 is set
    // (0x41a1d0).
    static void RemoveChain(GuiSide *side);
    // Resets the side's front for a new round (0x41a230).
    void Reset();
    // Sets unk_abcc and restarts timerAbd0 (0x415d50).
    void StartUnknownAbcc();
    // Starts script 0x43 on unk_59c8 (0x41a210).
    void StartUnknown59c8();
    // Starts scripts 0x44-0x47 on unk_5c6c and sets unk_a65c
    // (0x41a270).
    void StartUnknown5c6c();
    // Interrupts unk_5c6c with 1 and clears unk_a65c (0x41a2d0).
    void StopUnknown5c6c();
    // Starts scripts 0x48-0x4a on unk_66fc, the last showing digit
    // `digit` (sprite 0x35 on) (0x41a320).
    void StartUnknown66fc(int digit);
    // Starts script 0x4b on unk_6ee8[0] and 0x4c + `idx` on
    // unk_6ee8[1] (0x41a380).
    void StartUnknown6ee8(int idx);
    // Starts script 0x4e on unk_7430 (0x41a3c0).
    void StartUnknown7430();
    // Starts scripts 0x4f-0x54 on unk_76d4 and shows `frames` on them
    // with SetUnknown76d4Time (0x41a3e0).
    void StartUnknown76d4(int frames);
    // Shows `frames` as a time on unk_76d4's digits, clearing them
    // when it is negative (0x418950).
    void SetUnknown76d4Time(int frames);
    // Creates side `sideIdx`'s front, zeroed, and adds its update element
    // (priority 0x17 + `sideIdx`) and draw element (0x1a + `sideIdx`).
    // Returns it, or NULL if adding fails (0x41a5d0).
    static GuiSide *RegisterChain(int sideIdx);

    Sprite unk_0[0xb];
    Sprite unk_1d0c[5];
    Sprite unk_2a40[7];
    Sprite unk_3cbc;
    Sprite unk_3f60[10];
    Sprite unk_59c8;
    Sprite unk_5c6c[4];
    Sprite unk_66fc[3];
    Sprite unk_6ee8[2];
    Sprite unk_7430;
    Sprite unk_76d4[6];
    Sprite unk_86ac[7];
    Sprite unk_9928[5];
    // Set while unk_5c6c shows, by StartUnknown5c6c.
    int unk_a65c;
    ChainElem *calcElem;
    ChainElem *drawElem;
    // The side's ANM file.
    AnmLoaded *anm;
    int sideIdx;
    // The side, and the other one.
    GameSide *side;
    GameSide *otherSide;
    GuiSideA678 unk_a678;
    // Reset turns it from set to 2, restarting timerAbd0 at 30.
    int unk_abcc;
    Timer timerAbd0;
    Timer timerAbdc;
};
ASSERT_SIZE(GuiSide, 0xabe8);
