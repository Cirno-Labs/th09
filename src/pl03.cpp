// Character 3's extra attacks and callbacks (pl/pl03.cpp).

#include "effect.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"
#include "sprite.h"
#include "unsorted.h"

// The update callback of effect type 18: grows unknown7c by 0.7 up to
// 144, sets unknownC9 and puts the effect on its side's player. Returns 1
// (0x443250).
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

// The init callback of extra attack type 13, character 3's: type 0's
// sprite and curve, flying in to within 128 of the target's player,
// kept in the play area across and above its bottom. Returns 0
// (0x443440).
int ExAttackInit443440(ExAttack *attack)
{
    ExAttackData00 *data;

    attack->AllocSprites(1, sizeof(ExAttackData00));
    attack->GetSide()->effectManager->anm->StartScriptAtOrigin(attack->sprites, 0x25);
    data = (ExAttackData00 *)attack->data;
    attack->layer = 2;
    SetPolar((Vector2 *)&data->tangent1,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    SetPolar((Vector2 *)&data->tangent0,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    data->start.x = g_GameManager.ArenaToScreenX(attack->pos.x);
    data->start.y = g_GameManager.ArenaToScreenY(attack->pos.y);
    data->start.z = 0.0f;
    data->targetPos.x =
        g_Rng.GetRandomF32SignedInRange(128.0f) + g_GameManager.sides[attack->targetSide].player->pos.x;
    data->targetPos.y =
        g_Rng.GetRandomF32SignedInRange(128.0f) + g_GameManager.sides[attack->targetSide].player->pos.y;
    if (data->targetPos.y > 432.0f)
    {
        data->targetPos.y = 432.0f;
    }
    if (data->targetPos.x < -128.0f)
    {
        data->targetPos.x = -128.0f;
    }
    else if (data->targetPos.x > 128.0f)
    {
        data->targetPos.x = 128.0f;
    }
    data->targetPos.z = 0.0f;
    g_App.SetCamera(attack->targetSide);
    data->end.x = g_GameManager.ArenaToScreenX(data->targetPos.x);
    data->end.y = g_GameManager.ArenaToScreenY(data->targetPos.y);
    data->end.z = 0.0f;
    g_App.SetCamera(attack->side);
    return 0;
}

// The update callback of extra attack type 13: flies in along the curve
// for 180 frames, then appears on the target's side and, after 20
// frames, sits there as a hit circle shedding particles until it touches
// the player there or 300 frames pass. Returns 1 once it is done
// (0x4435e0).
int ExAttackUpdate4435e0(ExAttack *attack)
{
    ExAttackData00 *data = (ExAttackData00 *)attack->data;
    D3DXVECTOR3 size(24.0f, 24.0f, 0.0f);

    switch (data->state)
    {
    case 0:
        HermiteInterpolate(&attack->pos, &data->end, &data->start, &data->tangent0, &data->tangent1,
                           attack->timer.AsFloat(), 180.0f);
        if (attack->timer > 0xb4)
        {
            g_GameManager.sides[attack->side].player->anm->StartScriptAtOrigin(attack->sprites, 9);
            data->state++;
            attack->pos = data->targetPos;
            attack->layer = attack->targetSide;
            attack->timer = 0;
        }
        break;
    case 1:
        if (attack->timer.HasTickedEvery(2))
        {
            g_GameManager.sides[attack->targetSide].effectManager->SpawnParticles(9, &attack->pos, 1, 0xffffffff);
        }
        if (attack->timer > 0x14)
        {
            if (g_GameManager.sides[attack->targetSide].player->CheckHitAreas(&attack->pos, &size, NULL) == 2)
            {
                data->state = 2;
                attack->timer = 0;
                attack->sprites->SetPendingInterrupt(1);
            }
            else
            {
                g_GameManager.sides[attack->targetSide].player->unknown36c.AddCircle(&attack->pos, 14.0f, NULL);
            }
            if (attack->timer > 300)
            {
                data->state = 2;
                attack->timer = 0;
                attack->sprites->SetPendingInterrupt(1);
                return 0;
            }
        }
        break;
    case 2:
        if (attack->timer > 0x14)
        {
            return 1;
        }
        break;
    }
    return 0;
}

// The combo func of character 3: while unknown34 reaches 70 -
// unknownB4 * 3, sends an extra attack of type 13 from `pos` and takes
// 70 off it. Returns 0 (0x443790).
int PlayerComboFunc443790(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= 70 - g_GameManager.rank.unknownB4 * 3)
    {
        g_GameManager.exAttackManager->AddAttack(13, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= 70;
    }
    return 0;
}

// The draw callback of effect type 18: DrawEffectType10's fan
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
