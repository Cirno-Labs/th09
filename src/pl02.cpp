// Character 2's extra attacks and callbacks (pl/pl02.cpp).

#include "effect.h"
#include "enemy.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"
#include "unsorted.h"

// The init callback of effect type 16: EffectInit447c40's ring of
// radius 16, with unknown84 at 0.01. Returns 0, or -1 if the allocation
// fails (0x441c80).
int EffectInit441c80(Effect *effect)
{
    effect->vertices = (VertexTex1DiffuseXyzrwh *)g_MemoryManager.Alloc(0x294, "Effect");
    if (effect->vertices == NULL)
    {
        return -1;
    }
    effect->unknown48 = effect->pos;
    effect->unknown54.x = 0.0f;
    effect->unknown54.y = 0.0f;
    effect->unknown54.z = 1.0f;
    effect->unknown60.x = 0.0f;
    effect->unknown60.y = -1.0f;
    effect->unknown60.z = 0.0f;
    effect->unknown80 = effect->velocity.x;
    effect->unknown88 = effect->velocity.z;
    effect->unknown94 = 0.0f;
    effect->unknown98 = 0.0f;
    effect->unknown9c = 1.0f;
    effect->unknown8c = 0x20;
    effect->unknown7c = 16.0f;
    effect->unknownC9 = 1;
    effect->unknown84 = 0.01f;
    return 0;
}

// An update callback of the effect table: after 30 frames, grows
// unknown84 by 0.001 up to 0.08; sets unknownC9 and puts the effect on
// its side's player. Returns 1 (0x441d30).
int EffectUpdate441d30(Effect *effect)
{
    effect->unknownC9 = 1;
    if (effect->unknownAc >= 0x1e)
    {
        if (effect->unknown84 < 0.08f)
        {
            effect->unknown84 += 0.001f;
        }
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    return 1;
}

// A fire callback of the shot table (1): on the frame of `sd`, sets the
// player's unknown3031c to the bullet for 30 frames, keeps the shot's
// option and offset in the bullet, then fires it as type 2 with an empty
// trail.
// Returns 1 if it fired (0x441ef0).
int ShotFire441ef0(Player *player, PlayerBullet *bullet, int frame, ShotData *sd)
{
    int i;

    if (frame != sd->frame)
    {
        return 0;
    }
    player->unknown3031c.timer = 30;
    player->unknown3031c.bullet = bullet;
    bullet->unknown468 = sd->option;
    bullet->velocity.z = sd->offsetX;
    bullet->unknown448 = sd->offsetY;
    bullet->unknown46a = 0xf;
    FireShot(player, bullet, sd);
    bullet->type = 2;
    for (i = 0x1f; i >= 0; i--)
    {
        bullet->trail[i].x = -999.0f;
    }
    bullet->pos.x = -999.0f;
    return 1;
}

// A draw callback of the shot table (18): draws the bullet's sprite at
// each point of its trail up to the first unused one, twice
// unknown46a of them, fading out along it. Returns 0 (0x442220).
int ShotDraw442220(Player *player, PlayerBullet *bullet)
{
    int alpha = bullet->sprite.alpha;
    int i;

    for (i = 0; i < bullet->unknown46a * 2; i++)
    {
        // -999.0f, compared by its bits.
        if (*(DWORD *)&bullet->trail[i].x == 0xc479c000)
        {
            break;
        }
        bullet->sprite.pos.x = g_GameManager.ArenaToScreenX(bullet->trail[i].x);
        bullet->sprite.pos.y = g_GameManager.ArenaToScreenY(bullet->trail[i].y);
        bullet->sprite.pos.z = 0.0f;
        bullet->sprite.alpha = alpha - alpha * i / (bullet->unknown46a * 2);
        g_SpriteEngine->Draw(&bullet->sprite);
    }
    bullet->sprite.alpha = alpha;
    return 0;
}

// The update callback of effect type 17: grows unknown84 by pi/60 up to
// pi/2, sets unknownC9, puts the effect on its side's player and turns
// unknown80 a quarter of the shorter way towards opposite the player's
// unknown1cd8. Returns 1 (0x4422f0).
int EffectUpdate4422f0(Effect *effect)
{
    float target;
    float delta;

    effect->unknownC9 = 1;
    if (effect->unknown84 < 1.5707964f)
    {
        effect->unknown84 += 0.05235988f;
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    target = AddNormalizeAngle(effect->mgr->side->player->unknown1cd8, 3.1415927f);
    delta = target - effect->unknown80;
    if (target > effect->unknown80)
    {
        if (delta > 3.1415927f)
        {
            delta -= 6.2831855f;
        }
    }
    else
    {
        if (delta < -3.1415927f)
        {
            delta += 6.2831855f;
        }
    }
    effect->unknown80 = AddNormalizeAngle(effect->unknown80, delta * 0.25f);
    return 1;
}

// A fire callback of the shot table (2): fires `sd` on its frame,
// standing still at its angle. Returns 1 if it fired (0x4423d0).
int ShotFire4423d0(Player *player, PlayerBullet *bullet, int frame, ShotData *sd)
{
    if (frame == sd->frame)
    {
        FireShot(player, bullet, sd);
        bullet->velocity.x = 0.0f;
        bullet->velocity.y = 0.0f;
        bullet->angle = sd->angle;
        return 1;
    }
    return 0;
}

// The unknown30404 of characters 2 and 10: whether `pos` is within the
// radius of the player's unknown368 and the arc of unknown84 radians
// about its angle unknown80 (0x442480).
int PlayerPosFunc442480(Player *player, D3DXVECTOR3 *pos)
{
    D3DXVECTOR3 diff = player->pos - *pos;
    Effect *circle = player->unknown368;
    float angle;

    if (diff.x * diff.x + diff.y * diff.y < circle->unknown7c * circle->unknown7c)
    {
        angle = player->AngleFromPlayer(pos) - circle->unknown80;
        if (Fabs(angle) > 3.1415927f)
        {
            angle = angle > 0.0f ? angle - 6.2831855f : angle + 6.2831855f;
        }
        if (Fabs(angle) <= circle->unknown84 * 0.5f)
        {
            return 1;
        }
    }
    return 0;
}

// The unknown30408 of character 2: stops the enemy's velocity2.
// Returns 0 (0x442550).
int PlayerEnemyFunc442550(Player *player, Enemy *enemy)
{
    enemy->velocity2.x = 0.0f;
    enemy->velocity2.y = 0.0f;
    enemy->velocity2.z = 0.0f;
    enemy->unknown5424++;
    return 0;
}

// The free callback of extra attack type 4: frees the data block, which
// its init allocates apart from the sprites. Returns 0 (0x442ec0).
int ExAttackFree442ec0(ExAttack *attack)
{
    if (attack->data != NULL)
    {
        g_MemoryManager.Free(attack->data);
    }
    attack->data = NULL;
    return 0;
}

// The unknown30450 of character 2: while unknown34 reaches
// (45 - unknownB4) * 2, sends an extra attack of type 3 from `pos` and
// takes as much off it. Returns 0 (0x442ee0).
int PlayerComboFunc442ee0(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= (45 - g_GameManager.unknownB4) * 2)
    {
        g_GameManager.exAttackManager->AddAttack(3, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= (45 - g_GameManager.unknownB4) * 2;
    }
    return 0;
}

// The init callback of effect type 17: a ring of 16 segments, radius
// 160, around its position, from opposite the player's unknown1cd8, as
// wide as velocity.z, in 0x294 bytes of vertices. Returns 0, or -1 if
// the allocation fails (0x442f50).
int EffectInit442f50(Effect *effect)
{
    effect->vertices = (VertexTex1DiffuseXyzrwh *)g_MemoryManager.Alloc(0x294, "Effect");
    if (effect->vertices == NULL)
    {
        return -1;
    }
    effect->unknown48 = effect->pos;
    effect->unknown54.x = 0.0f;
    effect->unknown54.y = 0.0f;
    effect->unknown54.z = 1.0f;
    effect->unknown60.x = 0.0f;
    effect->unknown60.y = -1.0f;
    effect->unknown60.z = 0.0f;
    effect->unknown80 = AddNormalizeAngle(effect->mgr->side->player->unknown1cd8, 3.1415927f);
    effect->unknown94 = 0.0f;
    effect->unknown98 = 0.0f;
    effect->unknown84 = 0.0f;
    effect->unknown88 = effect->velocity.z;
    effect->unknown8c = 0x10;
    effect->unknown9c = 1.0f;
    effect->unknown7c = 160.0f;
    effect->unknownC9 = 1;
    return 0;
}

// An update callback of the effect table: grows unknown7c by 0.7 up to
// 144.0, sets unknownC9 and puts the effect on its side's player. Returns
// 1 (0x443250).
int EffectUpdate443250(Effect *effect)
{
    effect->unknownC9 = 1;
    if (effect->unknown7c < 144.0f)
    {
        effect->unknown7c += 0.7f;
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    return 1;
}
