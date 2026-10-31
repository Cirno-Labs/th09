#include "bullet.h"
#include "enemy.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"
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

int *ResolveOperand(Enemy *enemy, int *operand, WORD mask, int bit)
{
    if (bit >= 0 && (mask & (1 << bit)) == 0)
    {
        return operand;
    }
    switch (*operand)
    {
    case 10000:
        return &enemy->currentCtx->intVars[0];
    case 10001:
        return &enemy->currentCtx->intVars[1];
    case 10002:
        return &enemy->currentCtx->intVars[2];
    case 10003:
        return &enemy->currentCtx->intVars[3];
    case 10004:
        return &enemy->currentCtx->intVars[4];
    case 10005:
        return &enemy->currentCtx->intVars[5];
    case 10006:
        return &enemy->currentCtx->intVars[6];
    case 10007:
        return &enemy->currentCtx->intVars[7];
    case 10008:
        return &enemy->enemyInts[0];
    case 10009:
        return &enemy->enemyInts[1];
    case 10010:
        return &enemy->enemyInts[2];
    case 10011:
        return &enemy->enemyInts[3];
    case 10012:
        return &enemy->enemyInts[4];
    case 10013:
        return &enemy->enemyInts[5];
    case 10014:
        return &enemy->enemyInts[6];
    case 10015:
        return &enemy->enemyInts[7];
    case 10053:
        return &enemy->currentCtx->args.ints[0];
    case 10054:
        return &enemy->currentCtx->args.ints[1];
    case 10055:
        return &enemy->currentCtx->args.ints[2];
    case 10056:
        return &enemy->currentCtx->args.ints[3];
    case 10036:
        return &enemy->currentCtx->intVars2[0];
    case 10037:
        return &enemy->currentCtx->intVars2[1];
    case 10038:
        return &enemy->currentCtx->intVars2[2];
    case 10039:
        return &enemy->currentCtx->intVars2[3];
    case 10040:
        return &g_GameManager.difficulty;
    case 10041:
        return &g_GameManager.unknownB4;
    case 10049:
        return &enemy->callbackTimer.current;
    case 10051:
        return &enemy->life;
    case 10092:
        return &enemy->item;
    case 10093:
        return &enemy->score;
    case 10061:
        return &enemy->manager->ecl.callArgs.ints[0];
    case 10062:
        return &enemy->manager->ecl.callArgs.ints[1];
    case 10063:
        return &enemy->manager->ecl.callArgs.ints[2];
    case 10064:
        return &enemy->manager->ecl.callArgs.ints[3];
    }
    return operand;
}

float *ResolveFloatOperand(Enemy *enemy, float *operand, WORD mask, int bit)
{
    if (bit >= 0 && (mask & (1 << bit)) == 0)
    {
        return operand;
    }
    switch ((int)*operand)
    {
    case 10016:
        return &enemy->currentCtx->floatVars[0];
    case 10017:
        return &enemy->currentCtx->floatVars[1];
    case 10018:
        return &enemy->currentCtx->floatVars[2];
    case 10019:
        return &enemy->currentCtx->floatVars[3];
    case 10020:
        return &enemy->currentCtx->floatVars[4];
    case 10021:
        return &enemy->currentCtx->floatVars[5];
    case 10022:
        return &enemy->currentCtx->floatVars[6];
    case 10023:
        return &enemy->currentCtx->floatVars[7];
    case 10024:
        return &enemy->enemyFloats[0];
    case 10025:
        return &enemy->enemyFloats[1];
    case 10026:
        return &enemy->enemyFloats[2];
    case 10027:
        return &enemy->enemyFloats[3];
    case 10028:
        return &enemy->enemyFloats[4];
    case 10029:
        return &enemy->enemyFloats[5];
    case 10030:
        return &enemy->enemyFloats[6];
    case 10031:
        return &enemy->enemyFloats[7];
    case 10057:
        return &enemy->currentCtx->args.floats[0];
    case 10058:
        return &enemy->currentCtx->args.floats[1];
    case 10059:
        return &enemy->currentCtx->args.floats[2];
    case 10060:
        return &enemy->currentCtx->args.floats[3];
    case 10042:
        return &enemy->pos.x;
    case 10043:
        return &enemy->pos.y;
    case 10044:
        return &enemy->pos.z;
    case 10045:
        return &enemy->manager->side->player->pos.x;
    case 10046:
        return &enemy->manager->side->player->pos.y;
    case 10047:
        return &enemy->manager->side->player->pos.z;
    case 10094:
        return &enemy->currentCtx->floatVars2[0];
    case 10095:
        return &enemy->currentCtx->floatVars2[1];
    case 10065:
        return &enemy->manager->ecl.callArgs.floats[0];
    case 10066:
        return &enemy->manager->ecl.callArgs.floats[1];
    case 10067:
        return &enemy->manager->ecl.callArgs.floats[2];
    case 10068:
        return &enemy->manager->ecl.callArgs.floats[3];
    case 10074:
        return &enemy->moveStart.x;
    case 10075:
        return &enemy->moveStart.y;
    case 10076:
        return &enemy->moveStart.z;
    case 10079:
        return &enemy->moveDelta.x;
    case 10080:
        return &enemy->moveDelta.y;
    case 10081:
        return &enemy->moveDelta.z;
    case 10069:
        return &enemy->angle;
    case 10070:
        return &enemy->angularVelocity;
    case 10071:
        return &enemy->speed;
    case 10072:
        return &enemy->acceleration;
    case 10073:
        return &enemy->orbitRadius;
    case 10077:
        return &enemy->orbitAngle;
    case 10078:
        return &enemy->orbitAngularVelocity;
    }
    return operand;
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
