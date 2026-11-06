// What the characters' objects (pl00.cpp to pl15.cpp) share, outside
// the precompiled header: the compiler compiles these inline functions
// where each object includes them, before its own functions, as the
// original has them at the start of pl00's and pl01's code.

#pragma once

#include "enemy.h"
#include "exattack.h"
#include "gamemanager.h"

inline Enemy *EnemyManager::SpawnPlayerEnemy(int sub, D3DXVECTOR3 *pos, int life, int itemDrop, int score,
                                             int mirrored)
{
    return this->SpawnEnemy(sub, pos, life, itemDrop, score, mirrored, 1);
}

inline Enemy *EnemyManager::SpawnPlayerEnemyWithVars(int sub, D3DXVECTOR3 *pos, int life, char itemDrop, int score,
                                                     EnemyScriptVars *vars)
{
    return this->SpawnEnemyWithVars(sub, pos, life, itemDrop, score, vars, 1);
}

inline GameSide *ExAttack::GetSide()
{
    return &g_GameManager.sides[this->side];
}
