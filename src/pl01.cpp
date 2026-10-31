// Character 1's spell cards, extra attacks and callbacks (pl/pl01.cpp).

#include "effect.h"
#include "enemy.h"
#include "gamemanager.h"
#include "player.h"
#include "spellcard.h"

// The unknown30404 of character 1: whether `pos` is above the player,
// within a parabola about it that the player's unknown368 widens by its
// unknown84, and at least 14 across (0x4417f0).
int PlayerPosFunc4417f0(Player *player, D3DXVECTOR3 *pos)
{
    float top = player->pos.y + 32.0f;
    float dx;
    float width;

    if (top <= pos->y)
    {
        return 0;
    }
    top = pos->y - (top - 448.0f);
    dx = pos->x - player->pos.x;
    width = (200704.0f - top * top) * player->unknown368->unknown84 * player->unknown368->unknown84;
    if (width < 196.0f)
    {
        width = 196.0f;
    }
    if (dx * dx <= width)
    {
        return 1;
    }
    return 0;
}

// A hit callback of the shot table (22): counts unknown30354 up.
// Returns 0 (0x441880).
int ShotHit441880(Player *player, PlayerBullet *bullet, D3DXVECTOR3 *pos)
{
    player->unknown30354++;
    return 0;
}

