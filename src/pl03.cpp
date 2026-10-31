// Character 3's extra attacks and callbacks (pl/pl03.cpp).

#include "effect.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"
#include "sprite.h"
#include "unsorted.h"

// A fire callback of the shot table (3): fires `sd` on its frame, going
// up at 24, and sets the player's unknown1cdc and unknown1ce0 to 0.5.
// Returns 1 if it fired (0x4432a0).
int ShotFire4432a0(Player *player, PlayerBullet *bullet, int frame, ShotData *sd)
{
    if (frame == sd->frame)
    {
        FireShot(player, bullet, sd);
        bullet->velocity.x = 0.0f;
        bullet->velocity.y = -24.0f;
        player->unknown1cdc = player->unknown1ce0 = 0.5f;
        return 1;
    }
    return 0;
}

// A draw callback of the shot table (19): draws the bullet's sprite
// once for each of the first four points of its trail up to the first
// unused one, shrinking and brightening from a quarter of its color,
// without moving it. Returns 0 (0x443300).
int ShotDraw443300(Player *player, PlayerBullet *bullet)
{
    int red;
    int green;
    int blue;
    int alpha;
    int i;

    blue = bullet->sprite.colorRgb[2];
    green = bullet->sprite.colorRgb[1];
    red = bullet->sprite.colorRgb[0];
    alpha = bullet->sprite.alpha;

    for (i = 0; i < 4; i++)
    {
        // -999.0f, compared by its bits.
        if (*(DWORD *)&bullet->trail[i].x == 0xc479c000)
        {
            break;
        }
        bullet->sprite.scaleY = 3.0f - i * 0.33333334f;
        bullet->sprite.scaleX = 4.0f - i * 0.33333334f;
        bullet->sprite.colorRgb[2] = blue * i / 4 + 0x3f;
        bullet->sprite.colorRgb[1] = green * i / 4 + 0x3f;
        bullet->sprite.alpha = alpha * i / 4 + 0x3f;
        g_SpriteEngine->Draw(&bullet->sprite);
    }
    bullet->sprite.colorRgb[2] = blue;
    bullet->sprite.colorRgb[1] = green;
    bullet->sprite.colorRgb[0] = red;
    bullet->sprite.alpha = alpha;
    return 0;
}

// A hit callback of the shot table (23): does nothing. Returns 1
// (0x443430).
int ShotHit443430(Player *player, PlayerBullet *bullet, D3DXVECTOR3 *pos)
{
    return 1;
}

// The unknown30450 of character 3: while unknown34 reaches 70 -
// unknownB4 * 3, sends an extra attack of type 13 from `pos` and takes
// 70 off it. Returns 0 (0x443790).
int PlayerComboFunc443790(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= 70 - g_GameManager.unknownB4 * 3)
    {
        g_GameManager.exAttackManager->AddAttack(13, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= 70;
    }
    return 0;
}

// The draw callback of effect type 18: EffectDraw4414a0's fan
// (0x4437f0).
int EffectDraw4437f0(Effect *effect)
{
    if (effect->unknownC9)
    {
        float delta = 6.2831855f / (effect->unknown8c - 1);
        VertexDiffuseXyzrwh *vtx = (VertexDiffuseXyzrwh *)effect->vertices;
        float angle = -3.1415927f;
        int i;

        vtx->pos = effect->pos;
        vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
        vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
        vtx->w = 1.0f;
        vtx->diffuse = 0x40600040;
        vtx++;
        for (i = effect->unknown8c; i > 0; i--)
        {
            if (angle >= 3.1415927f)
            {
                angle -= 6.2831855f;
            }
            vtx->pos.z = 0.0f;
            SetPolar((Vector2 *)&vtx->pos, angle, effect->unknown7c);
            vtx->pos += effect->pos;
            vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
            vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
            vtx->w = 1.0f;
            vtx->diffuse = 0x70600040;
            vtx++;
            angle += delta;
        }
    }
    g_SpriteEngine->DrawColoredFan(effect->sprites, (VertexDiffuseXyzrwh *)effect->vertices, effect->unknown8c + 1);
    return 1;
}

// An update callback of the shot table (13): keeps the bullet on the
// player, raised by its velocity.y as that falls by 1 a frame, and sets
// the player's unknown1cdc and unknown1ce0 to 1 after 15 frames. While
// it flies, adds a damage box and a hit box above it that widen with
// its age. Returns 0 (0x443930).
int ShotUpdate443930(Player *player, PlayerBullet *bullet)
{
    bullet->pos = player->pos;
    bullet->pos.y += bullet->velocity.y;
    bullet->velocity.y -= 1.0f;
    if (bullet->timer >= 15)
    {
        player->unknown1cdc = player->unknown1ce0 = 1.0f;
    }
    if (bullet->state == 1)
    {
        float width = bullet->timer.AsFloat() + 80.0f;
        D3DXVECTOR3 pos = bullet->pos;

        pos.y -= 16.0f;
        pos.x -= (width - 96.0f) * 0.5f;
        player->AddDamageBox(&pos, width, 40.0f, 15, 0, 0);
        player->AddHitBox(&pos, width, 30.0f, 0, 0);
    }
    return 0;
}

// The update callback of effect type 26: grows unknown7c by 8 up to 112,
// sets unknownC9, puts the effect on its side's player and turns
// unknown80 an eighth of the shorter way towards opposite the player's
// unknown1cd8, as EffectUpdate4422f0. Returns 1 (0x443a30).
int EffectUpdate443a30(Effect *effect)
{
    float target;
    float delta;

    effect->unknownC9 = 1;
    if (effect->unknown7c < 112.0f)
    {
        effect->unknown7c += 8.0f;
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
    effect->unknown80 = AddNormalizeAngle(effect->unknown80, delta * 0.125f);
    return 1;
}
