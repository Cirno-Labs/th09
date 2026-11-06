// The effects, one manager per side ("EffectSysInf").

#pragma once

#include <d3dx8.h>

#include "chain.h"
#include "timer.h"

struct GameSide;
class AnmLoaded;
class EffectManager;
class Sprite;
struct VertexTex1DiffuseXyzrwh;

// An effect (0xd8 bytes). Only the fields known so far.
struct Effect
{
    // The effect's manager.
    EffectManager *mgr;
    // The effect's own sprites, spriteCount of them, allocated by
    // StartEffect.
    Sprite *sprites;
    int spriteCount;
    D3DXVECTOR3 pos;
    // As spawned; some types' init callbacks take an angle in x.
    D3DXVECTOR3 velocity;
    unsigned char pad_24[0x3c - 0x24];
    // Set by the init callbacks: EffectInitRing's start position and
    // two directions, InitEffectTypes1And2's two random offsets in 3c and
    // 48, its start position in 54 and velocity on screen in 60.
    D3DXVECTOR3 unk_3c;
    D3DXVECTOR3 unk_48;
    D3DXVECTOR3 unk_54;
    D3DXVECTOR3 unk_60;
    unsigned char pad_6c[0x7c - 0x6c];
    // Taken from the first sprite by UpdateEffectFromSprite: its posOffset.x,
    // rotation.z, scaleX, scriptVar0, posOffset.y, rotation.y and
    // scriptVar1.
    float unk_7c;
    float unk_80;
    float unk_84;
    float unk_88;
    int unk_8c;
    // The reserved slot it was started in, by SpawnReservedEffect.
    int reservedSlot;
    float unk_94;
    float unk_98;
    float unk_9c;
    float unk_a0;
    // UpdateEffectTypes3And4 spawns enemy subroutine unk_a4 on the side
    // other than unk_a8.
    short unk_a4;
    short unk_a6;
    short unk_a8;
    short unk_aa;
    Timer timer;
    unsigned char pad_b8[0xbc - 0xb8];
    // From the effect's EffectInfo.
    int (*update)(Effect *effect);
    int (*draw)(Effect *effect);
    // Whether the effect runs.
    BYTE active;
    // The type it was started as.
    BYTE type;
    // Set to have the effect fade out, as th08's.
    BYTE fadeOut;
    unsigned char pad_c7[0xc8 - 0xc7];
    // OnUpdate puts the effect on draw list unk_114 when set, or
    // drawList2 when unk_ca is.
    BYTE unk_c8;
    BYTE unk_c9;
    BYTE unk_ca;
    unsigned char pad_cb[0xcc - 0xcb];
    // Vertices the effect allocated, freed when its slot is reused, as
    // th08's.
    VertexTex1DiffuseXyzrwh *vertices;
    // Set by UpdateEffectTypes1And2 while it waits out unk_98.
    BYTE unk_d0;
    unsigned char pad_d1[0xd4 - 0xd1];
    // The next effect of a list.
    Effect *next;
};
ASSERT_SIZE(Effect, 0xd8);

// An entry of the effect table (0x4a0b48), by effect type, as th08's
// EffectInfo.
struct EffectInfo
{
    // The script of the first sprite, and how far the next sprite's is.
    int script;
    int scriptStep;
    int spriteCount;
    // Called once the effect is started; it is stopped unless this
    // returns 0.
    int (*init)(Effect *effect);
    int (*update)(Effect *effect);
    int (*draw)(Effect *effect);
};
ASSERT_SIZE(EffectInfo, 0x18);
extern EffectInfo g_EffectInfos[];

// A side's effect manager (0x2dc bytes); the third side has one too. Only
// the fields known so far.
class EffectManager
{
public:
    // Zeroes the manager. Inline; the copy is at 0x40c950.
    void Init()
    {
        memset(this, 0, sizeof(EffectManager));
    }
    // Zeroes the manager, then sets colorScale to 1. Inline; the copy is
    // at 0x40d2e0.
    EffectManager()
    {
        this->Init();
        this->colorScale[0] = 1.0f;
        this->colorScale[1] = 1.0f;
        this->colorScale[2] = 1.0f;
        this->colorScale[3] = 1.0f;
    }

    // Frees the blocks of each effect and the effects. Returns 0. Clears
    // vertices where it should clear sprites (0x40d200).
    static int FreeEffects(EffectManager *mgr);
    // Frees the effects, removes the manager's chain elements and deletes
    // it, if any (0x40d280).
    static void RemoveChain(EffectManager *mgr);
    // Resets the effects for a new round (0x40c990).
    void Reset();
    // Frees `effect`'s blocks and starts it as effect `effectIdx` at
    // `pos`, with `velocity`, its sprites from g_EffectInfos in `color`.
    // Returns it (0x40ca60).
    Effect *StartEffect(Effect *effect, int effectIdx, D3DXVECTOR3 *pos, D3DCOLOR color, D3DXVECTOR3 *velocity);
    // Spawns `count` of effect `effectIdx` at `pos` in `color`, standing
    // still, in the first unk_34 effects, as th08's
    // SpawnEffectWithVelocity. Returns the last effect started, or the
    // last of all effects if not all could be (0x40cc00).
    Effect *SpawnEffect(int effectIdx, D3DXVECTOR3 *pos, int count, D3DCOLOR color);
    // SpawnEffect with `velocity`, as th08's SpawnEffectWithVelocity
    // (0x40ccc0).
    Effect *SpawnEffectWithVelocity(int effectIdx, D3DXVECTOR3 *pos, D3DXVECTOR3 *velocity, int count,
                                       D3DCOLOR color);
    // Starts effect `effectIdx` at `pos` in `color`, standing still, in
    // reserved slot `slot`, after the first unk_34 effects, as th08's
    // SpawnReservedEffect. Returns it (0x40cd70).
    Effect *SpawnReservedEffect(int effectIdx, D3DXVECTOR3 *pos, int slot, D3DCOLOR color);
    // The update and draw chain callbacks; the third side's draws with
    // OnDraw2 (0x40cdd0, 0x40cfb0, 0x40d070).
    static int OnUpdate(EffectManager *mgr);
    static int OnDraw(EffectManager *mgr);
    static int OnDraw2(EffectManager *mgr);
    // Draws the second list's effects, with their draw callbacks or their
    // sprites at their position in the play area; BulletManager::OnDraw
    // calls it. Returns 1 (0x40d110).
    static int DrawUnknown114(EffectManager *mgr);
    // The update element's added callback: takes etama.anm from ANM
    // slot 8, loading it if needed. Returns 0 (0x40d1c0).
    static int AddedCallback(EffectManager *mgr);
    // Creates side `sideIdx`'s effect manager with `count34` + `count38`
    // + 1 effects and adds its update element (priority 0x13 +
    // `sideIdx`) and draw element (the same, or 0x1e for the third
    // side). Returns it, or NULL if adding fails (0x40d320).
    static EffectManager *RegisterChain(int sideIdx, int count34, int count38);
    // Reserved slot `slot`, taking unk_34 as 256. Inline; the copy is
    // at 0x445560.
    Effect *GetReservedEffect(int slot)
    {
        return &this->effects[slot + 0x100];
    }

    // Where SpawnEffect starts looking for a free effect.
    int next;
    unsigned char pad_4[0x8 - 0x4];
    // How many effects are active, counted by OnUpdate.
    int activeCount;
    int sideIdx;
    // The side, and the other one; for the third side, both itself.
    GameSide *side;
    GameSide *otherSide;
    ChainElem *calcElem;
    ChainElem *drawElem;
    float colorScale[4];
    Effect *effects;
    // The effects are unk_34 + unk_38 + 1.
    int unk_34;
    int unk_38;
    // The heads of three lists of effects; OnDraw2 draws the first.
    Effect drawList0;
    Effect drawList1;
    Effect drawList2;
    // The last effects of the draw lists unk_3c, unk_114 and
    // drawList2.
    Effect *drawListTails[3];
    // How many frames OnUpdate ran.
    int frames;
    AnmLoaded *anm;
    unsigned char pad_2d8[0x2dc - 0x2d8];
};
ASSERT_SIZE(EffectManager, 0x2dc);

// An effect init callback of the effect table: moves the effect 256
// along the angle in velocity.x and turns its first sprite to the angle
// plus pi/2. Returns 0 (0x40e080).
int InitOrientedEffect(Effect *effect);

// An effect update callback of the effect table: puts the effect on its
// side's player, as th08's EffectUpdateFollowPlayer. Returns 1
// (0x40c960).
int EffectUpdateFollowPlayer(Effect *effect);

// An effect update callback of the effect table: takes the first
// sprite's offset, rotation, y scale and script variables into the
// effect and sets unk_c9. Returns 1 (0x40d470).
int UpdateEffectFromSprite(Effect *effect);

// The init callback of effect type 8: allocates 0x1c38 bytes of
// vertices, filled from the first sprite, and takes the position and
// velocity's parts into the effect. Returns 0, or -1 if the allocation
// fails (0x40dbb0).
int EffectInitRing(Effect *effect);

// An init callback of the effect table: keeps the start position and the
// velocity moved to the screen, and picks two random offsets and
// unk_9c. Returns 0 (0x40d520).
int InitEffectTypes1And2(Effect *effect);

// An init callback of the effect table: InitEffectTypes1And2 without
// clearing unk_98. Returns 0 (0x40d910).
int InitEffectTypes3And4(Effect *effect);

// An update callback of the effect table: moves the effect along a
// Hermite curve for unk_9c, then spawns enemy subroutine unk_a4
// at its velocity on the other side, aimed near the player there or
// straight down. Returns 1 while moving, else 0 (0x40d9e0).
int UpdateEffectTypes3And4(Effect *effect);

// An update callback of the effect table: while unk_98 is set, waits
// it out and plays sound 0x2e; else moves the effect along a Hermite
// curve for unk_9c, then fires a bullet at its velocity on the other
// side. Returns 1 while waiting or moving, else 0 (0x40d600).
int UpdateEffectTypes1And2(Effect *effect);

// The draw callback of the ring effects, as th08's EffectDrawRing: when
// unk_c9 is set, rebuilds the ring of unk_8c segments, unk_88
// wide, at radius unk_7c from angle unk_80 around unk_48 (an
// ellipse of y radius unk_94 turned by unk_98, or with unk_9c
// waves of unk_94 if those are set). Returns 1 (0x40dc70).
int EffectDrawRing(Effect *effect);
