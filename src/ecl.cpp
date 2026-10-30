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

void ScriptMoveLinear(Enemy *enemy, EclInstr *instr, float t)
{
    float start;
    float end;

    start = GetFloatOperand(enemy, instr->args[4].f);
    end = GetFloatOperand(enemy, instr->args[5].f);
    *ResolveFloatOperand(enemy, &instr->args[8].f, 0, -1) = (end - start) * t + start;
}

void ScriptMoveHermite(Enemy *enemy, EclInstr *instr, float t)
{
    float start;
    float hStart;
    float weightIn;
    float end;
    float slopeIn;
    float wEnd;
    float tangentOut;
    float outWeight;

    start = GetFloatOperand(enemy, instr->args[4].f);
    end = GetFloatOperand(enemy, instr->args[5].f);
    slopeIn = GetFloatOperand(enemy, instr->args[6].f);
    tangentOut = GetFloatOperand(enemy, instr->args[7].f);
    hStart = (t - 1.0f) * (t - 1.0f) * (2.0f * t + 1.0f);
    wEnd = t * t * (3.0f - 2.0f * t);
    weightIn = (1.0f - t) * (1.0f - t) * t;
    outWeight = (t - 1.0f) * t * t;
    *ResolveFloatOperand(enemy, &instr->args[8].f, 0, -1) =
        hStart * start + wEnd * end + weightIn * slopeIn + outWeight * tangentOut;
}

void SendExAttack1(Enemy *enemy, EclInstr *instr)
{
    g_GameManager.exAttackManager->AddAttack(1, &enemy->pos, 1 - enemy->manager->sideIdx, NULL);
}

void SetSidesFlag0(Enemy *enemy, EclInstr *instr)
{
    enemy->manager->side->flag0 = instr->args[1].i;
    enemy->manager->otherSide->flag0 = instr->args[1].i;
    if (instr->args[1].i != 0)
    {
        g_SoundPlayer.PlaySoundByIdx(0x34, 0);
    }
}

void SendExAttack7(Enemy *enemy, EclInstr *instr)
{
    g_GameManager.exAttackManager->AddAttack(7, &enemy->pos, 1 - enemy->manager->sideIdx, NULL);
}

void SendExAttack8(Enemy *enemy, EclInstr *instr)
{
    g_GameManager.exAttackManager->AddAttack(8, &enemy->pos, 1 - enemy->manager->sideIdx, NULL);
}

void SendExAttack9(Enemy *enemy, EclInstr *instr)
{
    g_GameManager.exAttackManager->AddAttack(9, &enemy->pos, 1 - enemy->manager->sideIdx, NULL);
}

void SendExAttack23(Enemy *enemy, EclInstr *instr)
{
    g_GameManager.exAttackManager->AddAttack(23, &enemy->pos, 1 - enemy->manager->sideIdx, NULL);
}

void SendExAttack24(Enemy *enemy, EclInstr *instr)
{
    g_GameManager.exAttackManager->AddAttack(24, &enemy->pos, 1 - enemy->manager->sideIdx, NULL);
}

void SendExAttack10(Enemy *enemy, EclInstr *instr)
{
    D3DXVECTOR3 pos;

    pos.x = enemy->currentCtx->floatVars2[0];
    pos.y = enemy->currentCtx->floatVars2[1];
    pos.z = 0.0f;
    g_GameManager.exAttackManager->AddAttack(10, &pos, 1 - enemy->manager->sideIdx, NULL);
}

void SendExAttack11(Enemy *enemy, EclInstr *instr)
{
    D3DXVECTOR3 pos;

    pos.x = enemy->currentCtx->floatVars2[0];
    pos.y = enemy->currentCtx->floatVars2[1];
    pos.z = enemy->currentCtx->floatVars[0];
    g_GameManager.exAttackManager->AddAttack(11, &pos, 1 - enemy->manager->sideIdx, NULL);
}

void SendExAttack12(Enemy *enemy, EclInstr *instr)
{
    D3DXVECTOR3 pos;

    pos.x = enemy->currentCtx->floatVars2[0];
    pos.y = enemy->currentCtx->floatVars2[1];
    pos.z = enemy->currentCtx->floatVars[0];
    g_GameManager.exAttackManager->AddAttack(12, &pos, 1 - enemy->manager->sideIdx, NULL);
}

void SendExAttack22(Enemy *enemy, EclInstr *instr)
{
    D3DXVECTOR3 target;

    ((Vector2 *)&target)->SetPolar(1.5707964f, 64.0f);
    target += enemy->pos;
    target.z = 1.5707964f;
    g_GameManager.exAttackManager->AddAttack(22, &enemy->pos, 1 - enemy->manager->sideIdx, &target);
    ((Vector2 *)&target)->SetPolar(1.2566371f, 64.0f);
    target += enemy->pos;
    target.z = 1.2566371f;
    g_GameManager.exAttackManager->AddAttack(22, &enemy->pos, 1 - enemy->manager->sideIdx, &target);
    ((Vector2 *)&target)->SetPolar(1.8849556f, 64.0f);
    target += enemy->pos;
    target.z = 1.8849556f;
    g_GameManager.exAttackManager->AddAttack(22, &enemy->pos, 1 - enemy->manager->sideIdx, &target);
}
