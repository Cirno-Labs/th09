#include "bullet.h"
#include "enemy.h"
#include "exattack.h"
#include "gamemanager.h"
#include "unsorted.h"

void BulletManager::Unknown405650()
{
    this->Unknown412590(1);
}

int EclManager::Load(char *path)
{
    int i;

    this->file = (EclHeader *)OpenPath(path, NULL, 0);
    if (this->file == NULL)
    {
        LogPrintf(&g_Log, "敵データの読み込みに失敗しました、データが壊れてるか失われています\r\n");
        return -1;
    }
    if (this->file->version != 0x900)
    {
        LogPrintf(&g_Log, "敵データのバージョンが違います\r\n");
        return -1;
    }
    for (i = 0; i < this->file->timelineCount; i++)
    {
        this->file->timelines[i] = (char *)this->file->timelines[i] + (int)this->file;
    }
    this->subs = &this->file->timelines[this->file->timelineCount];
    for (i = 0; i < this->file->subCount; i++)
    {
        this->subs[i] = (char *)this->subs[i] + (int)this->file;
    }
    return 0;
}

void EclManager::Unload()
{
    if (this->file != NULL)
    {
        g_MemoryManager.Free(this->file);
    }
    this->file = NULL;
}

int EclManager::StartSub(EnemyUnknown22c *ctx, short sub)
{
    if (sub < 0)
    {
        return 0;
    }
    ctx->code = this->subs[sub];
    ctx->timer = 0;
    ctx->waitTimer = 0;
    ctx->sub = sub;
    ctx->ecl = this;
    return 0;
}

void SendExAttack1(Enemy *enemy, EclInstr *instr)
{
    g_GameManager.exAttackManager->AddAttack(1, &enemy->pos, 1 - enemy->manager->sideIdx, 0);
}

void SendExAttack7(Enemy *enemy, EclInstr *instr)
{
    g_GameManager.exAttackManager->AddAttack(7, &enemy->pos, 1 - enemy->manager->sideIdx, 0);
}

void SendExAttack8(Enemy *enemy, EclInstr *instr)
{
    g_GameManager.exAttackManager->AddAttack(8, &enemy->pos, 1 - enemy->manager->sideIdx, 0);
}

void SendExAttack9(Enemy *enemy, EclInstr *instr)
{
    g_GameManager.exAttackManager->AddAttack(9, &enemy->pos, 1 - enemy->manager->sideIdx, 0);
}

void SendExAttack23(Enemy *enemy, EclInstr *instr)
{
    g_GameManager.exAttackManager->AddAttack(23, &enemy->pos, 1 - enemy->manager->sideIdx, 0);
}

void SendExAttack24(Enemy *enemy, EclInstr *instr)
{
    g_GameManager.exAttackManager->AddAttack(24, &enemy->pos, 1 - enemy->manager->sideIdx, 0);
}

void SendExAttack10(Enemy *enemy, EclInstr *instr)
{
    D3DXVECTOR3 pos;

    pos.x = enemy->currentCtx->floatVars2[0];
    pos.y = enemy->currentCtx->floatVars2[1];
    pos.z = 0.0f;
    g_GameManager.exAttackManager->AddAttack(10, &pos, 1 - enemy->manager->sideIdx, 0);
}

void SendExAttack11(Enemy *enemy, EclInstr *instr)
{
    D3DXVECTOR3 pos;

    pos.x = enemy->currentCtx->floatVars2[0];
    pos.y = enemy->currentCtx->floatVars2[1];
    pos.z = enemy->currentCtx->floatVars[0];
    g_GameManager.exAttackManager->AddAttack(11, &pos, 1 - enemy->manager->sideIdx, 0);
}

void SendExAttack12(Enemy *enemy, EclInstr *instr)
{
    D3DXVECTOR3 pos;

    pos.x = enemy->currentCtx->floatVars2[0];
    pos.y = enemy->currentCtx->floatVars2[1];
    pos.z = enemy->currentCtx->floatVars[0];
    g_GameManager.exAttackManager->AddAttack(12, &pos, 1 - enemy->manager->sideIdx, 0);
}
