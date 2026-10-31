// The extra attacks the sides send each other ("ExAttackCtrlInf").

#pragma once

#include <d3dx8math.h>

#include "chain.h"
#include "gamemanager.h"
#include "sprite.h"
#include "timer.h"

struct ExAttack;
// An extra attack's callbacks, which take the attack.
typedef int (*ExAttackCallback)(ExAttack *attack);

// An extra attack (0x4c bytes). Only the fields known so far.
struct ExAttack
{
    // The side that sent it. Inline; the copy is at 0x440cb0.
    GameSide *GetSide()
    {
        return &g_GameManager.sides[this->side];
    }
    // Allocates `count` zeroed sprites with `dataSize` bytes of data
    // after them. Inline, in global.h; the copy is at 0x440d90.
    void AllocSprites(int count, int dataSize)
    {
        this->spriteCount = count;
        this->sprites = (Sprite *)g_MemoryManager.Alloc(count * sizeof(Sprite) + dataSize, "./system\\global.h");
        memset(this->sprites, 0, count * sizeof(Sprite) + dataSize);
        this->data = (BYTE *)this->sprites + count * sizeof(Sprite);
    }

    // Which of the three lists OnUpdate links it into.
    int layer;
    // The side it attacks and the side that sent it.
    int targetSide;
    int side;
    // Whether the attack runs.
    int active;
    Timer timer;
    // Its sprites, spriteCount of them, and data after them, in one
    // block AllocSprites allocates.
    Sprite *sprites;
    D3DXVECTOR3 pos;
    int spriteCount;
    // The next attack in its layer's list.
    ExAttack *next;
    void *data;
    // AddAttack's target.
    D3DXVECTOR3 *target;
    // The rotation its sprites that turn are drawn at.
    float angle;
    // Its type's callbacks: run every frame until it returns nonzero,
    // draw it, and run when the attack is freed, if set.
    ExAttackCallback onUpdate;
    ExAttackCallback onDraw;
    ExAttackCallback onFree;
};
ASSERT_SIZE(ExAttack, 0x4c);

// The extra attacks (0x4c80 bytes), at GameManager::unknownAc. Only the
// fields known so far.
class ExAttackManager
{
public:
    // Starts an attack of type `type` at `pos` from side `side`, some
    // types towards `target`, and plays sound 0x2d. Returns it, or the
    // spare attack if the side runs its limit or none is free; the
    // attack is stopped if its type's onInit fails (0x4151f0).
    ExAttack *AddAttack(int type, D3DXVECTOR3 *pos, int side, D3DXVECTOR3 *target);
    // Frees the attacks, removes the manager's chain elements and deletes
    // it, if any (0x4156a0).
    static void RemoveChain(ExAttackManager *mgr);
    // Ends and clears the attacks for a new round (0x4156f0).
    void Reset();
    // The update and draw chain callbacks (0x415340, 0x415460,
    // 0x415580).
    static int OnUpdate(ExAttackManager *mgr);
    static int OnDraw(ExAttackManager *mgr);
    static int OnDraw2(ExAttackManager *mgr);
    // The update element's added callback: sets limits by each side's
    // character and the difficulty, then resets the attacks. Returns 0
    // (0x415750).
    static int AddedCallback(ExAttackManager *mgr);
    // Creates the manager, zeroed, and adds its update element (priority
    // 0xe) and draw elements (0x10 and 0x1d). Returns it, or NULL if
    // adding fails (0x4157d0).
    static ExAttackManager *RegisterChain();

    ChainElem *calcElem;
    ChainElem *drawElem;
    ChainElem *drawElem2;
    // How many attacks each side runs, counted by OnUpdate, and how
    // many it may.
    int counts[2];
    int limits[2];
    // The loops stop before the last, which AddAttack hands out when
    // none is free.
    ExAttack attacks[0x101];
    // The first and last attack of each layer's list.
    ExAttack *layerHeads[3];
    ExAttack *layerTails[3];
};
ASSERT_SIZE(ExAttackManager, 0x4c80);

// An extra attack type's callbacks (0x4a0f78). The first starts the
// attack and returns nonzero if it fails.
struct ExAttackType
{
    ExAttackCallback onInit;
    ExAttackCallback onUpdate;
    ExAttackCallback onDraw;
    ExAttackCallback onFree;
};
extern ExAttackType g_ExAttackTypes[];

// Two numbers by character, which ExAttackManager::AddedCallback adds
// the first of to the second times the difficulty (0x4a1128, 0x4a117c).
extern int g_Unknown4a1128[];
extern int g_Unknown4a117c[];
