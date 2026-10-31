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
    // Set by the init callbacks: EffectInit40dbb0's start position and
    // two directions, EffectInit40d520's two random offsets in 3c and
    // 48, its start position in 54 and velocity on screen in 60.
    D3DXVECTOR3 unknown3c;
    D3DXVECTOR3 unknown48;
    D3DXVECTOR3 unknown54;
    D3DXVECTOR3 unknown60;
    unsigned char pad_6c[0x7c - 0x6c];
    // Taken from the first sprite by EffectUpdate40d470: its posOffset.x,
    // rotation.z, scaleY, scriptVar0, posOffset.y, rotation.y and
    // scriptVar1.
    float unknown7c;
    float unknown80;
    float unknown84;
    float unknown88;
    int unknown8c;
    // The reserved slot it was started in, by SpawnReservedEffect.
    int reservedSlot;
    float unknown94;
    float unknown98;
    float unknown9c;
    float unknownA0;
    // EffectUpdate40d9e0 spawns enemy subroutine unknownA4 on the side
    // other than unknownA8.
    short unknownA4;
    short unknownA6;
    short unknownA8;
    short unknownAa;
    Timer unknownAc;
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
    unsigned char pad_c7[0xc9 - 0xc7];
    BYTE unknownC9;
    unsigned char pad_ca[0xcc - 0xca];
    // Vertices the effect allocated, freed when its slot is reused, as
    // th08's.
    VertexTex1DiffuseXyzrwh *vertices;
    // Set by EffectUpdate40d600 while it waits out unknown98.
    BYTE unknownD0;
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
    // Zeroes the manager, then sets unknown20 to 1. Inline; the copy is
    // at 0x40d2e0.
    EffectManager()
    {
        this->Init();
        this->unknown20[0] = 1.0f;
        this->unknown20[1] = 1.0f;
        this->unknown20[2] = 1.0f;
        this->unknown20[3] = 1.0f;
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
    // still, in the first unknown34 effects, as th08's
    // SpawnEffectWithVelocity. Returns the last effect started, or the
    // last of all effects if not all could be (0x40cc00).
    Effect *SpawnParticles(int effectIdx, D3DXVECTOR3 *pos, int count, D3DCOLOR color);
    // SpawnParticles with `velocity`, as th08's SpawnEffectWithVelocity
    // (0x40ccc0).
    Effect *SpawnParticlesWithVelocity(int effectIdx, D3DXVECTOR3 *pos, D3DXVECTOR3 *velocity, int count,
                                       D3DCOLOR color);
    // Starts effect `effectIdx` at `pos` in `color`, standing still, in
    // reserved slot `slot`, after the first unknown34 effects, as th08's
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
    // Reserved slot `slot`, taking unknown34 as 256. Inline; the copy is
    // at 0x445560.
    Effect *GetReservedEffect(int slot)
    {
        return &this->effects[slot + 0x100];
    }

    // Where SpawnParticles starts looking for a free effect.
    int next;
    unsigned char pad_4[0xc - 0x4];
    int sideIdx;
    // The side, and the other one; for the third side, both itself.
    GameSide *side;
    GameSide *otherSide;
    ChainElem *calcElem;
    ChainElem *drawElem;
    float unknown20[4];
    Effect *effects;
    // The effects are unknown34 + unknown38 + 1.
    int unknown34;
    int unknown38;
    // The heads of three lists of effects; OnDraw2 draws the first.
    Effect unknown3c;
    Effect unknown114;
    Effect unknown1ec;
    unsigned char pad_2c4[0x2d4 - 0x2c4];
    AnmLoaded *anm;
    unsigned char pad_2d8[0x2dc - 0x2d8];
};
ASSERT_SIZE(EffectManager, 0x2dc);

// An effect init callback of the effect table: moves the effect 256
// along the angle in velocity.x and turns its first sprite to the angle
// plus pi/2. Returns 0 (0x40e080).
int EffectInit40e080(Effect *effect);

// An effect update callback of the effect table: puts the effect on its
// side's player, as th08's EffectUpdateFollowPlayer. Returns 1
// (0x40c960).
int EffectUpdateFollowPlayer(Effect *effect);

// An effect update callback of the effect table: takes the first
// sprite's offset, rotation, y scale and script variables into the
// effect and sets unknownC9. Returns 1 (0x40d470).
int EffectUpdate40d470(Effect *effect);

// The init callback of effect type 8: allocates 0x1c38 bytes of
// vertices, filled from the first sprite, and takes the position and
// velocity's parts into the effect. Returns 0, or -1 if the allocation
// fails (0x40dbb0).
int EffectInit40dbb0(Effect *effect);

// An init callback of the effect table: keeps the start position and the
// velocity moved to the screen, and picks two random offsets and
// unknown9c. Returns 0 (0x40d520).
int EffectInit40d520(Effect *effect);

// An init callback of the effect table: EffectInit40d520 without
// clearing unknown98. Returns 0 (0x40d910).
int EffectInit40d910(Effect *effect);

// An update callback of the effect table: moves the effect along a
// Hermite curve for unknown9c, then spawns enemy subroutine unknownA4
// at its velocity on the other side, aimed near the player there or
// straight down. Returns 1 while moving, else 0 (0x40d9e0).
int EffectUpdate40d9e0(Effect *effect);

// An update callback of the effect table: while unknown98 is set, waits
// it out and plays sound 0x2e; else moves the effect along a Hermite
// curve for unknown9c, then fires a bullet at its velocity on the other
// side. Returns 1 while waiting or moving, else 0 (0x40d600).
int EffectUpdate40d600(Effect *effect);

// The draw callback of the ring effects, as th08's EffectDrawRing: when
// unknownC9 is set, rebuilds the ring of unknown8c segments, unknown88
// wide, at radius unknown7c from angle unknown80 around unknown48 (an
// ellipse of y radius unknown94 turned by unknown98, or with unknown9c
// waves of unknown94 if those are set). Returns 1 (0x40dc70).
int EffectDrawRing(Effect *effect);
