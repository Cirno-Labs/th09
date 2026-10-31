#include "gui.h"
#include "ascii.h"
#include "gamemanager.h"
#include "network.h"
#include "player.h"
#include "screeneffect.h"
#include "spellcard.h"
#include "sprite.h"
#include "stage.h"

// Here rather than with the GameManager: the object before gui.cpp's
// (after PlayRecord::AddClear5).
void GameManager::PickOpponent()
{
    GameManagerUnknownF0 *opponents[16];
    GameManagerUnknownF0 *opponent;
    int count;
    int total;
    short weight;
    int pick;
    int i;
    int n;

    if (this->IsMode0() || this->IsMode1())
    {
        opponent = g_StoryOpponents[this->sides[0].character];
        count = 0;
        total = 0;
        for (; opponent->round >= 0; opponent++)
        {
            if (opponent->round == this->unknownFc)
            {
                total += opponent->weight;
                opponents[count] = opponent;
                count++;
            }
        }
        weight = g_Rng.GetRandomU32InRange(total);
        for (i = 0; i < count; i++)
        {
            if (weight < opponents[i]->weight && !this->unknownD8[opponents[i]->character])
            {
                break;
            }
            weight -= opponents[i]->weight;
        }
        if (i >= count)
        {
            for (i = 0; this->unknownD8[opponents[i]->character]; i++)
            {
            }
        }
        opponent = opponents[i];
        this->stage = opponent->stage;
        this->sides[1].character = opponent->character;
        g_PlayRecord.AddClear5(opponent->character);
        this->unknownD8[this->sides[1].character] = 1;
        this->unknownF0 = opponent;
        if (!this->sides[0].stats->unknown14 && this->sides[0].stats->lives > 0.0f)
        {
            this->sides[1].unknown2c = g_Unknown4a1500[this->difficulty][this->unknownFc][0];
            return;
        }
        this->sides[1].unknown2c =
            g_Unknown4a1500[this->difficulty][this->unknownFc][(g_GameManager.unknown100 > 3 ? 3 : g_GameManager.unknown100) + 1];
        return;
    }
    this->unknownFc = 9;
    if (this->unknown138 == -1)
    {
        pick = g_Rng.GetRandomU32InRange(2);
        for (opponent = g_MatchStages; opponent->round >= 0; opponent++)
        {
            if (opponent->character == this->sides[pick].character)
            {
                break;
            }
        }
    }
    else if (this->unknown138 == -2)
    {
        count = 0;
        for (i = 0; i <= 13; i++)
        {
            if (g_PlayRecord.openEntries[i] || g_Network->unknownA8)
            {
                count++;
            }
        }
        pick = g_Rng.GetRandomU32InRange(count);
        n = 0;
        for (i = 0; i < 16; i++)
        {
            if (g_PlayRecord.openEntries[i] || g_Network->unknownA8)
            {
                if (n == pick)
                {
                    break;
                }
                n++;
            }
        }
        for (opponent = g_MatchStages; opponent->round >= 0; opponent++)
        {
            if (opponent->character == i)
            {
                break;
            }
        }
    }
    else
    {
        for (opponent = g_MatchStages; opponent->round >= 0; opponent++)
        {
            if (opponent->character == this->unknown138)
            {
                break;
            }
        }
    }
    if (opponent->round < 0)
    {
        opponent = g_MatchStages;
    }
    this->sides[0].unknown2c = g_Unknown4a1884[this->difficulty];
    this->sides[1].unknown2c = g_Unknown4a1884[this->difficulty];
    this->stage = opponent->stage;
    this->unknownF0 = opponent;
}

int Gui::LoadAnm()
{
    return g_SpriteEngine->PreloadAnm(10, "front.anm") == NULL;
}

void Gui::ReleaseAnm()
{
    g_SpriteEngine->ReleaseAnm(10);
}

// Decodes a dialogue line from `src` into `dst`, as th08's
// DecodeMsgString but with a key that starts at 0x77 and grows by 7, 23,
// 39, ... Static, with src in eax (0x415c60).
static void DecodeMsgString(char *dst, char *src)
{
    int i;
    char c;
    BYTE key;
    BYTE step;

    i = 0;
    key = 0x77;
    step = 7;
    do
    {
        c = *src ^ key;
        dst[i] = c;
        i++;
        src++;
        key += step;
        step += 0x10;
    } while (c != 0);
}

void GuiMsg::Start(int index, int side)
{
    MsgFile *msgFile = this->msgFile;
    MsgFile *msgFile2 = this->msgFile2;

    memset(this, 0, sizeof(GuiMsg));
    this->msgFile = msgFile;
    this->msgFile2 = msgFile2;
    if (side == 1)
    {
        msgFile = msgFile2;
    }
    this->side = side;
    this->scriptIndex = index;
    this->instr = msgFile->scripts[index].script;
    this->text[0].scriptIndex = -1;
    this->text[1].scriptIndex = -1;
    this->fontSize = 24;
    this->textColors[0] = 0xe8f0ff;
    this->textColors[1] = 0xf0e8ff;
    this->textColors[2] = 0xe8d0ff;
    this->textColors[3] = 0xd8ffff;
    this->shadowColors[0] = 0;
    this->shadowColors[1] = 0;
    this->shadowColors[2] = 0;
    this->shadowColors[3] = 0;
    this->skippable = 1;
    this->minWaitFrames = 6;
    this->textBoxShown = 1;
    this->speaker = 0;
    this->newPage = 1;
    this->nextLine = 0;
    this->activePortrait = 0xff;
    g_App.textAnm->StartScript(&this->text[0], 0);
    g_App.textAnm->StartScript(&this->text[1], 1);
    this->text[0].fontWidth = this->text[0].fontHeight = this->fontSize;
    this->text[1].fontWidth = this->text[1].fontHeight = this->fontSize;
    g_SpriteEngine->RenderText(&this->text[0], this->textColors[this->speaker], this->shadowColors[this->speaker], " ");
    g_SpriteEngine->RenderText(&this->text[1], this->textColors[this->speaker], this->shadowColors[this->speaker], " ");
}

void GuiMsg::StartRandom(int side)
{
    int i;
    int roll;
    int first = g_GameManager.sides[1 - side].character * 10;
    MsgFile *msgFile = this->msgFile;
    MsgFile *msgFile2 = this->msgFile2;

    memset(this, 0, sizeof(GuiMsg));
    this->msgFile = msgFile;
    this->msgFile2 = msgFile2;
    if (side == 1)
    {
        msgFile = msgFile2;
    }
    this->side = side;
    roll = g_Rng.GetRandomU16InRange(0x100);
    for (i = first; i < first + 10; i++)
    {
        if (msgFile->scripts[i].script != NULL)
        {
            if (roll < msgFile->scripts[i].weight)
            {
                break;
            }
            roll -= msgFile->scripts[i].weight;
        }
    }
    if (i >= first + 10)
    {
        roll = g_Rng.GetRandomU16InRange(0x100);
        for (i = 210; i < 220; i++)
        {
            if (msgFile->scripts[i].script != NULL && i < msgFile->unknown0)
            {
                if (roll < msgFile->scripts[i].weight)
                {
                    break;
                }
                roll -= msgFile->scripts[i].weight;
            }
        }
        if (i >= 220 || i >= msgFile->unknown0)
        {
            i = 210;
        }
    }
    this->instr = msgFile->scripts[i].script;
    this->text[0].scriptIndex = -1;
    this->text[1].scriptIndex = -1;
    this->fontSize = 24;
    this->textColors[0] = 0xe8f0ff;
    this->textColors[1] = 0xf0e8ff;
    this->textColors[2] = 0xe8d0ff;
    this->textColors[3] = 0xd8ffff;
    this->shadowColors[0] = 0;
    this->shadowColors[1] = 0;
    this->shadowColors[2] = 0;
    this->shadowColors[3] = 0;
    this->skippable = 1;
    this->minWaitFrames = 6;
    this->textBoxShown = 1;
    this->speaker = 0;
    this->newPage = 1;
    this->nextLine = 0;
    this->activePortrait = 0xff;
    g_App.textAnm->StartScript(&this->text[0], 0);
    g_App.textAnm->StartScript(&this->text[1], 1);
    this->text[0].fontWidth = this->text[0].fontHeight = this->fontSize;
    this->text[1].fontWidth = this->text[1].fontHeight = this->fontSize;
    g_SpriteEngine->RenderText(&this->text[0], this->textColors[this->speaker], this->shadowColors[this->speaker], " ");
    g_SpriteEngine->RenderText(&this->text[1], this->textColors[this->speaker], this->shadowColors[this->speaker], " ");
}

int GuiMsg::Run()
{
    MsgArgs *args;

    if (this->scriptIndex < 0)
    {
        return -1;
    }
    if (this->timelineRelease > 0)
    {
        this->timelineRelease--;
    }
    if (this->skippable && g_PlayerInputs[2].IsHeld2(0x100))
    {
        this->timer = this->instr->time;
    }
    while (this->timer >= this->instr->time)
    {
        switch (this->instr->opcode)
        {
        case 0:
            this->scriptIndex = -1;
            return -1;
        case 23:
            g_GameManager.sides[this->instr->i].player->SetUnknown0(1);
            g_GameManager.sides[this->instr->i].player->unknown303c8 = 0;
            break;
        case 15:
        {
            MsgPortraitArgs *portraitArgs;
            int who;
            int i;

            portraitArgs = &this->instr->portraitArgs;
            if (portraitArgs->who >= 0)
            {
                who = this->side == 0 ? portraitArgs->who : 1 - portraitArgs->who;
                if (this->activePortrait != who)
                {
                    for (i = 0; i < 4; i++)
                    {
                        if (i != who)
                        {
                            this->portraits[i * 2].pendingInterrupt = 4;
                            this->portraits[i * 2 + 1].pendingInterrupt = 4;
                        }
                    }
                }
                this->portraits[who * 2].pendingInterrupt = 3;
                this->portraits[who * 2 + 1].pendingInterrupt = 3;
                this->activePortrait = who;
                this->speaker = who;
                if (portraitArgs->sprites[0] >= 0)
                {
                    g_GameManager.sides[this->side].gui->unknownA678.anm->SetSprite(
                        &this->portraits[this->side * 2],
                        g_GameManager.sides[this->side].player->unknown303fc + portraitArgs->sprites[0]);
                    g_GameManager.sides[this->side].gui->unknownA678.anm->SetSprite(
                        &this->portraits[this->side * 2 + 1],
                        g_GameManager.sides[this->side].player->unknown303fc + portraitArgs->sprites[0] + 9);
                }
                if (portraitArgs->sprites[1] >= 0)
                {
                    g_GameManager.sides[1 - this->side].gui->unknownA678.anm->SetSprite(
                        &this->portraits[(1 - this->side) * 2],
                        g_GameManager.sides[1 - this->side].player->unknown303fc + portraitArgs->sprites[1]);
                    g_GameManager.sides[1 - this->side].gui->unknownA678.anm->SetSprite(
                        &this->portraits[(1 - this->side) * 2 + 1],
                        g_GameManager.sides[1 - this->side].player->unknown303fc + portraitArgs->sprites[1] + 9);
                }
            }
            else
            {
                this->portraits[0].pendingInterrupt = 4;
                this->portraits[2].pendingInterrupt = 4;
                this->portraits[1].pendingInterrupt = 4;
                this->portraits[3].pendingInterrupt = 4;
            }
            this->newPage = 1;
            break;
        }
        case 17:
        {
            MsgPortraitArgs *portraitArgs2;
            int who2;
            int i;

            portraitArgs2 = &this->instr->portraitArgs;
            if (portraitArgs2->who < 2)
            {
                who2 = this->side == 0 ? portraitArgs2->who : 1 - portraitArgs2->who;
            }
            else
            {
                who2 = portraitArgs2->who;
            }
            if (this->activePortrait != who2)
            {
                for (i = 0; i < 4; i++)
                {
                    if (i != who2)
                    {
                        this->portraits[i * 2].pendingInterrupt = 4;
                        this->portraits[i * 2 + 1].pendingInterrupt = 4;
                    }
                }
            }
            this->portraits[who2 * 2].pendingInterrupt = 3;
            this->portraits[who2 * 2 + 1].pendingInterrupt = 3;
            this->activePortrait = who2;
            if (portraitArgs2->sprites[0] >= 0)
            {
                if (who2 < 2)
                {
                    g_GameManager.sides[who2].gui->unknownA678.anm->SetSprite(
                        &this->portraits[who2 * 2],
                        g_GameManager.sides[who2].player->unknown303fc + portraitArgs2->sprites[0]);
                    g_GameManager.sides[who2].gui->unknownA678.anm->SetSprite(
                        &this->portraits[who2 * 2 + 1],
                        g_GameManager.sides[who2].player->unknown303fc + portraitArgs2->sprites[0] + 9);
                }
                else
                {
                    g_GameManager.sides[0].gui->unknownA678.unknown4->SetSprite(&this->portraits[who2 * 2],
                                                                               portraitArgs2->sprites[0]);
                    g_GameManager.sides[0].gui->unknownA678.unknown4->SetSprite(&this->portraits[who2 * 2 + 1],
                                                                               portraitArgs2->sprites[0] + 6);
                }
            }
            this->speaker = who2;
            this->newPage = 1;
            break;
        }
        case 26:
            this->newPage = 1;
            this->speaker = this->instr->b;
            break;
        case 1:
        {
            short who3;
            int idx;

            who3 = this->instr->args.who;
            if (who3 < 2)
            {
                idx = this->side == 0 ? who3 : 1 - who3;
                g_GameManager.sides[idx].gui->unknownA678.anm->StartScript(
                    &this->portraits[idx * 2], g_GameManager.sides[idx].player->unknown303f8 + idx);
                g_GameManager.sides[idx].gui->unknownA678.anm->StartScript(
                    &this->portraits[idx * 2 + 1], g_GameManager.sides[idx].player->unknown303f8 + idx + 4);
            }
            else
            {
                g_GameManager.sides[0].gui->unknownA678.unknown4->StartScript(&this->portraits[who3 * 2], who3);
                g_GameManager.sides[0].gui->unknownA678.unknown4->StartScript(&this->portraits[who3 * 2 + 1],
                                                                             who3 + 4);
            }
            break;
        }
        case 2:
        {
            short who4;
            int idx2;

            args = &this->instr->args;
            if (args->who < 2)
            {
                idx2 = this->side == 0 ? args->who : 1 - args->who;
                g_GameManager.sides[idx2].gui->unknownA678.anm->SetSprite(
                    &this->portraits[idx2 * 2], args->index + g_GameManager.sides[idx2].player->unknown303fc);
                g_GameManager.sides[idx2].gui->unknownA678.anm->SetSprite(
                    &this->portraits[idx2 * 2 + 1], g_GameManager.sides[idx2].player->unknown303fc + args->index + 9);
            }
            else
            {
                who4 = args->who;
                g_GameManager.sides[0].gui->unknownA678.unknown4->SetSprite(&this->portraits[who4 * 2], args->index);
                g_GameManager.sides[0].gui->unknownA678.unknown4->SetSprite(&this->portraits[who4 * 2 + 1],
                                                                           args->index + 6);
            }
            break;
        }
        case 3:
        {
            char text[0x40];

            args = &this->instr->args;
            if (args->index == 0 && this->text[1].scriptIndex >= 0)
            {
                g_SpriteEngine->RenderText(&this->text[1], this->textColors[args->who], this->shadowColors[args->who],
                                           " ");
            }
            g_App.textAnm->StartScript(&this->text[args->index], args->index);
            this->text[args->index].fontHeight = this->fontSize;
            this->text[args->index].fontWidth = this->text[args->index].fontHeight;
            DecodeMsgString(text, args->text);
            g_SpriteEngine->RenderText(&this->text[args->index], this->textColors[args->who],
                                       this->shadowColors[args->who], text);
            this->waitFrames = 0;
            break;
        }
        case 16:
        {
            char text2[0x40];

            args = &this->instr->args;
            if (this->newPage)
            {
                if (this->text[1].scriptIndex >= 0)
                {
                    g_SpriteEngine->RenderText(&this->text[1], this->textColors[this->speaker],
                                               this->shadowColors[this->speaker], " ");
                }
                this->nextLine = 0;
            }
            g_App.textAnm->StartScript(&this->text[this->nextLine], this->nextLine);
            this->text[this->nextLine].fontHeight = this->fontSize;
            this->text[this->nextLine].fontWidth = this->text[this->nextLine].fontHeight;
            DecodeMsgString(text2, (char *)args);
            g_SpriteEngine->RenderText(&this->text[this->nextLine], this->textColors[this->speaker],
                                       this->shadowColors[this->speaker], text2);
            this->waitFrames = 0;
            this->newPage = 0;
            this->nextLine++;
            break;
        }
        case 4:
            if (!this->skippable || !g_PlayerInputs[2].IsHeld2(0x100))
            {
                if (!g_PlayerInputs[2].IsJustPressed2(1) || this->waitFrames < this->minWaitFrames)
                {
                    if (this->waitFrames >= this->instr->i)
                    {
                        this->newPage = 1;
                        this->minWaitFrames = 30;
                        break;
                    }
                    this->waitFrames++;
                    goto stepSprites;
                }
                this->newPage = 1;
                this->minWaitFrames = 8;
            }
            break;
        case 5:
            args = &this->instr->args;
            this->portraits[args->who * 2].pendingInterrupt = args->value;
            this->portraits[args->who * 2 + 1].pendingInterrupt = args->value;
            break;
        case 6:
            this->timelineRelease++;
            break;
        case 7:
        {
            int track;

            track = this->instr->i;
            if (track < 0)
            {
                g_App.StopBgm();
            }
            else if (!g_GameManager.IsMode2())
            {
                g_App.PlayMusic(track, 0);
            }
            else
            {
                g_App.PlayMusic(g_GameManager.unknownF0->unknownC, 0);
            }
            break;
        }
        case 8:
            g_GameManager.sides[1].gui->unknownA678.anm->StartScript(
                &this->unknown1a7c, g_GameManager.sides[1].player->unknown303f0 + 4);
            this->waitFrames = 0;
            break;
        case 9:
            g_GameManager.gui->ScoreRoundBonus();
            g_App.FadeOutBgm(4.0f);
            break;
        case 10:
            goto stepSprites;
        case 12:
            g_App.FadeOutBgm(4.0f);
            break;
        case 14:
            g_Unknown4a7c80 = ScreenEffect::StartEffect(5, 300, 0xffffff, 0, 0, 0x23, 2);
            g_App.redrawFrames = 302;
            break;
        case 11:
            if (g_GameManager.IsMode0())
            {
                if (this->side == 0)
                {
                    g_App.scene = 3;
                }
                else
                {
                    g_GameManager.isInContinueMenu = 1;
                }
            }
            else if (g_GameManager.IsMode1())
            {
                g_App.scene = 3;
            }
            else
            {
                g_GameManager.unknown13e = 1;
            }
            goto stepSprites;
        case 13:
            this->skippable = this->instr->b;
            break;
        case 18:
            this->textBoxShown = this->instr->b;
            break;
        case 24:
            this->boxTimer = 1;
            break;
        case 25:
            g_GameManager.sides[0].gui->StartUnknown59c8();
            g_GameManager.sides[1].gui->StartUnknown59c8();
            break;
        case 28:
            g_GameManager.sides[0].stage->waitLabel = this->instr->i;
            g_GameManager.sides[1].stage->waitLabel = this->instr->i;
            break;
        }
        this->instr = (MsgInstr *)((BYTE *)this->instr + this->instr->argSize + 4);
    }
    this->timer++;
stepSprites:
    g_SpriteEngine->Step(&this->portraits[0]);
    g_SpriteEngine->Step(&this->portraits[2]);
    g_SpriteEngine->Step(&this->portraits[1]);
    g_SpriteEngine->Step(&this->portraits[3]);
    g_SpriteEngine->Step(&this->portraits[4]);
    g_SpriteEngine->Step(&this->portraits[6]);
    g_SpriteEngine->Step(&this->portraits[5]);
    g_SpriteEngine->Step(&this->portraits[7]);
    g_SpriteEngine->Step(&this->text[0]);
    g_SpriteEngine->Step(&this->text[1]);
    g_SpriteEngine->Step(&this->unknown1a7c);
    if (this->timer < 60 && this->skippable && g_PlayerInputs[2].IsHeld2(0x100))
    {
        this->timer = 60;
    }
    if (this->boxTimer >= 1)
    {
        if (this->boxTimer < 60 && this->skippable && g_PlayerInputs[2].IsHeld2(0x100))
        {
            this->timer = 60;
        }
        this->boxTimer++;
    }
    return 0;
}

int GuiMsg::Draw()
{
    float height;

    if (this->scriptIndex < 0)
    {
        return -1;
    }
    if (this->speaker < 2)
    {
        g_SpriteEngine->DrawUnrotated(&this->portraits[6]);
        g_SpriteEngine->DrawUnrotated(&this->portraits[7]);
        g_SpriteEngine->DrawUnrotated(&this->portraits[4]);
        g_SpriteEngine->DrawUnrotated(&this->portraits[5]);
        g_SpriteEngine->DrawUnrotated(&this->portraits[0]);
        g_SpriteEngine->DrawUnrotated(&this->portraits[1]);
    }
    else if (this->speaker == 2)
    {
        g_SpriteEngine->DrawUnrotated(&this->portraits[6]);
        g_SpriteEngine->DrawUnrotated(&this->portraits[7]);
        g_SpriteEngine->DrawUnrotated(&this->portraits[0]);
        g_SpriteEngine->DrawUnrotated(&this->portraits[1]);
        g_SpriteEngine->DrawUnrotated(&this->portraits[4]);
        g_SpriteEngine->DrawUnrotated(&this->portraits[5]);
    }
    else
    {
        g_SpriteEngine->DrawUnrotated(&this->portraits[4]);
        g_SpriteEngine->DrawUnrotated(&this->portraits[5]);
        g_SpriteEngine->DrawUnrotated(&this->portraits[0]);
        g_SpriteEngine->DrawUnrotated(&this->portraits[1]);
        g_SpriteEngine->DrawUnrotated(&this->portraits[6]);
        g_SpriteEngine->DrawUnrotated(&this->portraits[7]);
    }
    g_SpriteEngine->DrawUnrotatedMirrored(&this->portraits[2]);
    g_SpriteEngine->DrawUnrotatedMirrored(&this->portraits[3]);
    if (this->boxTimer < 40)
    {
        height = this->boxTimer.AsFloat() * 1.5f;
    }
    else
    {
        height = 60.0f;
    }
    if (height > 0.0f)
    {
        GuiUnknownE86c vertices[4];
        vertices[0].pos = D3DXVECTOR3(64.0f, 364.0f, 0.0f);
        vertices[1].pos = D3DXVECTOR3(576.0f, 364.0f, 0.0f);
        vertices[2].pos = D3DXVECTOR3(64.0f, 364.0f + height, 0.0f);
        vertices[3].pos = D3DXVECTOR3(576.0f, 364.0f + height, 0.0f);
        vertices[0].diffuse = vertices[1].diffuse = 0xd0000000;
        vertices[2].diffuse = vertices[3].diffuse = 0x90000000;
        vertices[0].w = vertices[1].w = vertices[2].w = vertices[3].w = 1.0f;
        g_SpriteEngine->FlushQuadBatch();
        if (this->textBoxShown)
        {
            g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
            g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
            g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
            g_App.SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
            g_App.d3dDevice->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
            g_App.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(GuiUnknownE86c));
            g_SpriteEngine->ClearVertexFormat();
            g_SpriteEngine->ClearColorOp();
            g_SpriteEngine->ClearBlendMode();
            g_SpriteEngine->ClearZWrite();
            g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
            g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
            g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        }
    }
    g_SpriteEngine->DrawUnrotated(&this->text[0]);
    g_SpriteEngine->DrawUnrotated(&this->text[1]);
    g_SpriteEngine->DrawUnrotated(&this->unknown1a7c);
    return 0;
}

void GuiMsg::UnloadBgm(int firstScript)
{
    int i;
    BYTE unloaded[0x100];

    if (this->msgFile == NULL)
    {
        return;
    }
    memset(unloaded, 0, sizeof(unloaded));
    if (!g_GameManager.IsMode2())
    {
        for (i = firstScript; i < firstScript + 10; i++)
        {
            if (i >= this->msgFile->unknown0)
            {
                break;
            }
            this->instr = this->msgFile->scripts[i].script;
            if (this->instr == NULL)
            {
                continue;
            }
            while (this->instr->opcode != 0)
            {
                if (this->instr->opcode == 7 && !unloaded[this->instr->i])
                {
                    g_App.UnloadBgm(this->instr->i);
                    unloaded[this->instr->i] = 1;
                }
                this->instr = (MsgInstr *)((BYTE *)this->instr + this->instr->argSize + 4);
            }
        }
    }
    else
    {
        g_App.UnloadBgm(g_GameManager.unknownF0->unknownC);
    }
    if (this->msgFile2 != NULL)
    {
        g_MemoryManager.Free(this->msgFile2);
    }
    g_MemoryManager.Free(this->msgFile);
    g_MsgFile = NULL;
    g_MsgFile2 = NULL;
    this->Clear();
}

int Gui::OnUpdate(Gui *gui)
{
    int i;
    int j;
    D3DXVECTOR3 pos;

    g_SpriteEngine->Step(&gui->unknownC[0]);
    g_SpriteEngine->Step(&gui->unknownC[1]);
    g_SpriteEngine->Step(&gui->unknownC[2]);
    g_SpriteEngine->Step(&gui->unknownC[3]);
    g_SpriteEngine->Step(&gui->unknownC[4]);
    g_SpriteEngine->Step(&gui->unknownC[5]);
    g_SpriteEngine->Step(&gui->unknownC[6]);
    g_SpriteEngine->Step(&gui->unknown11eac[0]);
    g_SpriteEngine->Step(&gui->unknown11eac[1]);
    g_SpriteEngine->Step(&gui->unknown106b4);
    if (g_BgmUnknown08 != gui->unknown10958)
    {
        gui->unknown10958 = g_BgmUnknown08;
        if (g_BgmUnknown08 >= 0)
        {
            gui->anm->StartScriptAtOrigin(&gui->unknown106b4, 0x57);
            gui->anm->SetSprite(&gui->unknown106b4, g_BgmUnknown08 + 0x52);
        }
    }
    for (i = 0; i < 2; i++)
    {
        if (gui->unknownE93c[i] > 0)
        {
            if (gui->unknownE93c[i] & 1)
            {
                for (j = 0; j < 5; j++)
                {
                    gui->unknownE86c[i][j].diffuse = gui->unknownE934[i];
                }
            }
            else
            {
                for (j = 0; j < 5; j++)
                {
                    gui->unknownE86c[i][j].diffuse = 0xffffffff;
                }
            }
            gui->unknownE93c[i]--;
        }
        else
        {
            for (j = 0; j < 5; j++)
            {
                gui->unknownE86c[i][j].diffuse = 0x80ffffff;
            }
        }
    }
    gui->msg.Run();
    if (gui->unknown1095c == 0)
    {
        return 1;
    }
    g_SpriteEngine->Step(&gui->unknown10968[0]);
    g_SpriteEngine->Step(&gui->unknown10968[1]);
    g_SpriteEngine->Step(&gui->unknown10968[2]);
    if (gui->unknown1095c >= 2)
    {
        g_SpriteEngine->Step(&gui->unknown10968[3]);
        g_SpriteEngine->Step(&gui->unknown10968[4]);
        g_SpriteEngine->Step(&gui->unknown10968[5]);
        g_SpriteEngine->Step(&gui->unknown10968[6]);
        if (gui->msg.scriptIndex < 0)
        {
            if (g_GameManager.IsMode0())
            {
                if (gui->unknown10964 == 0)
                {
                    if (g_GameManager.unknownFc >= 8)
                    {
                        g_App.scene = g_GameManager.IsReplay() ? 1 : 9;
                    }
                    else
                    {
                        g_App.scene = 3;
                    }
                }
                else
                {
                    g_GameManager.isInContinueMenu = 1;
                }
            }
            else if (g_GameManager.IsMode1())
            {
                if (gui->unknown10964 == 0)
                {
                    if (g_GameManager.unknownFc >= 8)
                    {
                        g_App.scene = g_GameManager.IsReplay() ? 1 : 9;
                    }
                    else
                    {
                        g_App.scene = 3;
                    }
                }
                else
                {
                    g_GameManager.isInContinueMenu = 1;
                }
            }
            else
            {
                g_GameManager.unknown13e = 1;
            }
        }
    }
    else if (gui->unknown10960 == 0)
    {
        g_GameManager.unknownBits134_11 = 1 << (1 - gui->unknown10964);
        g_GameManager.sides[1 - gui->unknown10964].spellCard->unknown98 = 0;
        pos.x = 0.0f;
        pos.y = 224.0f;
        pos.z = 0.0f;
        g_GameManager.sides[0].player->AddHitCircle(&pos, 500.0f, 0.0f, 30000, 0);
        g_GameManager.sides[0].player->AddKind4Circle(&pos, 500.0f, 0.0f, 200, 30000, 0);
        g_GameManager.sides[1].player->AddHitCircle(&pos, 500.0f, 0.0f, 30000, 0);
        g_GameManager.sides[1].player->AddKind4Circle(&pos, 500.0f, 0.0f, 200, 30000, 0);
        g_GameManager.sides[0].player->unknown30410.EndCombo();
        g_GameManager.sides[1].player->unknown30410.EndCombo();
    }
    else if (gui->unknown10960 == 30)
    {
        g_SoundPlayer.PlaySoundByIdx(0x31, 0);
        g_GameManager.sides[1 - gui->unknown10964].stage->tintFadeMode = 1;
        g_GameManager.sides[1 - gui->unknown10964].stage->tintFadeFrames = 30;
        ScreenEffect::StartEffect(1, 60, 16, 0, 0, 0x23, 1 - gui->unknown10964);
        ScreenEffect::StartEffect(1, 60, 4, 0, 0, 0x23, gui->unknown10964);
    }
    else if (gui->unknown10960 == 60)
    {
        if (!g_GameManager.IsMode2())
        {
            if (gui->unknown10964 != 0 && g_GameManager.sides[0].stats->lives > 0.0f)
            {
                g_GameManager.sides[0].stats->lives -= 1.0f;
                g_GameManager.sides[0].stats->unknown34 += 1.0f;
                gui->unknown11ea4 = 1;
            }
            else if (gui->unknown10964 == 0)
            {
                gui->msg.Start(g_GameManager.unknownF0->unknown8, 0);
            }
            else
            {
                gui->msg.Start(g_GameManager.unknownF0->unknownA, 0);
            }
        }
        else if (g_GameManager.unknown108[0] < g_GameManager.unknown104 &&
                 g_GameManager.unknown108[1] < g_GameManager.unknown104)
        {
            gui->unknown11ea4 = 1;
        }
        else
        {
            gui->msg.StartRandom(gui->unknown10964);
        }
    }
    else if (gui->unknown11ea4 != 0 && gui->unknown10960 == 160)
    {
        g_GameManager.sides[0].gui->StartUnknownAbcc();
        g_GameManager.sides[1].gui->StartUnknownAbcc();
    }
    else if (gui->unknown11ea4 != 0 && gui->unknown10960 == 190)
    {
        GameManager::ResetRound();
        if (g_GameManager.IsMode0() || g_GameManager.IsMode1())
        {
            if (g_GameManager.unknown100 >= 4 && (g_GameManager.stateFlags & 0x4000))
            {
                g_GameManager.sides[1].unknown2c = g_Unknown4a1500[g_GameManager.difficulty][g_GameManager.unknownFc][4];
            }
            else
            {
                g_GameManager.sides[1].unknown2c =
                    g_Unknown4a1500[g_GameManager.difficulty][g_GameManager.unknownFc][g_GameManager.unknown100 + 1];
            }
        }
    }
    gui->unknown10960++;
    return 1;
}

int Gui::OnDraw(Gui *gui)
{
    g_App.SetView2D(2);
    g_SpriteEngine->DrawUnrotated(&gui->unknownC[0]);
    g_SpriteEngine->DrawUnrotated(&gui->unknownC[1]);
    g_SpriteEngine->DrawUnrotated(&gui->unknownC[2]);
    g_SpriteEngine->DrawUnrotated(&gui->unknownC[3]);
    g_SpriteEngine->DrawUnrotated(&gui->unknownC[4]);
    g_SpriteEngine->DrawUnrotated(&gui->unknownC[5]);
    g_SpriteEngine->DrawUnrotated(&gui->unknownC[6]);
    g_SpriteEngine->DrawUnrotated(&gui->enemyMarkers[0]);
    g_SpriteEngine->DrawUnrotated(&gui->enemyMarkers[1]);
    DrawColoredLineStrip((VertexDiffuseXyzrwh *)gui->unknownE86c[0], 4);
    DrawColoredLineStrip((VertexDiffuseXyzrwh *)gui->unknownE86c[1], 4);
    gui->msg.Draw();
    if (gui->unknown1095c != 0)
    {
        D3DXVECTOR3 pos;

        pos.z = 0.0f;
        g_SpriteEngine->DrawUnrotated(&gui->unknown10968[0]);
        g_App.SetView2D(0);
        gui->unknown10968[1].pos.x = g_GameManager.ArenaToScreenX(gui->unknown10968[1].posOffset.x);
        gui->unknown10968[1].pos.y = g_GameManager.ArenaToScreenY(gui->unknown10968[1].posOffset.y);
        g_SpriteEngine->DrawUnrotated(&gui->unknown10968[1]);
        if (gui->unknown1095c >= 2 && gui->unknown10964 == 0)
        {
            gui->unknown10968[3].pos.x = g_GameManager.ArenaToScreenX(gui->unknown10968[3].posOffset.x);
            gui->unknown10968[3].pos.y = g_GameManager.ArenaToScreenY(gui->unknown10968[3].posOffset.y);
            g_SpriteEngine->DrawUnrotated(&gui->unknown10968[3]);
            gui->unknown10968[4].pos.x = g_GameManager.ArenaToScreenX(gui->unknown10968[4].posOffset.x);
            gui->unknown10968[4].pos.y = g_GameManager.ArenaToScreenY(gui->unknown10968[4].posOffset.y);
            g_SpriteEngine->DrawUnrotated(&gui->unknown10968[4]);
            gui->unknown10968[5].pos.x = g_GameManager.ArenaToScreenX(gui->unknown10968[5].posOffset.x);
            gui->unknown10968[5].pos.y = g_GameManager.ArenaToScreenY(gui->unknown10968[5].posOffset.y);
            g_SpriteEngine->DrawUnrotated(&gui->unknown10968[5]);
            gui->unknown10968[6].pos.x = g_GameManager.ArenaToScreenX(gui->unknown10968[6].posOffset.x);
            gui->unknown10968[6].pos.y = g_GameManager.ArenaToScreenY(gui->unknown10968[6].posOffset.y);
            g_SpriteEngine->DrawUnrotated(&gui->unknown10968[6]);
            pos.x = 224.0f;
            pos.y = 226.0f;
        }
        g_App.SetView2D(1);
        gui->unknown10968[2].pos.x = g_GameManager.ArenaToScreenX(gui->unknown10968[2].posOffset.x);
        gui->unknown10968[2].pos.y = g_GameManager.ArenaToScreenY(gui->unknown10968[2].posOffset.y);
        g_SpriteEngine->DrawUnrotated(&gui->unknown10968[2]);
        if (gui->unknown1095c >= 2)
        {
            if (gui->unknown10964 == 1)
            {
                gui->unknown10968[3].pos.x = g_GameManager.ArenaToScreenX(gui->unknown10968[3].posOffset.x);
                gui->unknown10968[3].pos.y = g_GameManager.ArenaToScreenY(gui->unknown10968[3].posOffset.y);
                g_SpriteEngine->DrawUnrotated(&gui->unknown10968[3]);
                gui->unknown10968[6].pos.x = g_GameManager.ArenaToScreenX(gui->unknown10968[6].posOffset.x);
                gui->unknown10968[6].pos.y = g_GameManager.ArenaToScreenY(gui->unknown10968[6].posOffset.y);
                g_SpriteEngine->DrawUnrotated(&gui->unknown10968[6]);
                pos.x = 544.0f;
                pos.y = 226.0f;
            }
            if (gui->unknown1095c >= 2 && gui->unknown10960 > 20)
            {
                g_AsciiText.AddFormat(&pos, "%7d", gui->unknown11e88);
                pos.y += 16.0f;
                g_AsciiText.AddFormat(&pos, "%7d", gui->unknown11e8c);
                pos.y += 16.0f;
                g_AsciiText.AddFormat(&pos, "%7d", gui->unknown11e90);
                pos.y += 16.0f;
                g_AsciiText.AddFormat(&pos, "%7d", gui->unknown11e94);
                pos.y += 16.0f;
                g_AsciiText.AddFormat(&pos, "%7d", gui->unknown11e98);
                pos.y += 16.0f;
                pos.x -= 9.0f;
                if (g_GameManager.unknownFc == 8)
                {
                    g_AsciiText.AddFormat(&pos, "%8d", gui->unknown11e9c);
                }
                pos.y += 32.0f;
                g_AsciiText.AddFormat(&pos, "%8d", gui->bonusTotal);
            }
        }
    }
    g_App.SetView2D(2);
    if (g_GameManager.IsMode2())
    {
        D3DXVECTOR3 timePos(297.0f, 0.0f, 0.0f);
        g_AsciiText.AddFormat(&timePos, "%.2d:%.2d", g_GameManager.rank.unknownD0 / 3600,
                              g_GameManager.rank.unknownD0 / 60 % 60);
    }
    else
    {
        D3DXVECTOR3 timePos(260.0f, 0.0f, 0.0f);
        g_AsciiText.AddFormat(&timePos, "STAGE %d %.2d:%.2d", g_GameManager.unknownFc + 1,
                              g_GameManager.rank.unknownD0 / 3600, g_GameManager.rank.unknownD0 / 60 % 60);
    }
    g_SpriteEngine->DrawUnrotated(&gui->unknown106b4);
    g_SpriteEngine->DrawUnrotated(&gui->unknown11eac[0]);
    g_SpriteEngine->DrawUnrotated(&gui->unknown11eac[1]);
    return 1;
}

void Gui::CountRoundWin(int side)
{
    this->unknown11ea8 = 1;
    g_GameManager.unknown100++;
    if (++g_GameManager.unknown108[side] >= g_GameManager.unknown104)
    {
        this->anm->StartScriptAtOrigin(&this->unknown10968[0], 0x3c);
        this->anm->StartScriptAtOrigin(&this->unknown10968[1], 0x3d);
        this->anm->StartScriptAtOrigin(&this->unknown10968[2], 0x3e);
        this->unknown10968[1].posOffset.x = -999.0f;
        this->unknown10968[2].posOffset.x = -999.0f;
        this->unknown10968[1].pos.x = -999.0f;
        this->unknown10968[2].pos.x = -999.0f;
        this->anm->SetSprite(&this->unknown10968[1 + side], 0x2c);
        this->anm->SetSprite(&this->unknown10968[2 - side], 0x2d);
    }
    this->unknown10964 = side;
    this->unknown1095c = 1;
    this->unknown10960 = 0;
    this->unknown11ea4 = 0;
}

void Gui::ScoreRoundBonus()
{
    int side;

    this->anm->StartScriptAtOrigin(&this->unknown10968[3], 0x3f);
    this->anm->StartScriptAtOrigin(&this->unknown10968[6], 0x42);
    side = this->unknown10964;
    this->unknown10960 = 0;
    this->unknown1095c = 2;
    this->unknown11e88 = g_GameManager.sides[side].player->unknownA8 * 100000;
    if (g_GameManager.sides[side].player->unknown30410.maxCount < 1000)
    {
        this->unknown11e8c = g_GameManager.sides[side].player->unknown30410.maxCount * 10000;
    }
    else
    {
        this->unknown11e8c = 9999990;
    }
    if (g_GameManager.sides[side].player->unknownB0 > 66)
    {
        this->unknown11e90 = 9999990;
    }
    else
    {
        this->unknown11e90 = g_GameManager.sides[side].player->unknownB0 * 150000;
    }
    if (g_GameManager.sides[side].player->unknownB4 > 33)
    {
        this->unknown11e94 = 9999990;
    }
    else
    {
        this->unknown11e94 = g_GameManager.sides[side].player->unknownB4 * 300000;
    }
    if (g_GameManager.sides[side].player->unknownB8 > 33)
    {
        this->unknown11e98 = 9999990;
    }
    else
    {
        this->unknown11e98 = g_GameManager.sides[side].player->unknownB8 * 300000;
    }
    if (g_GameManager.unknownFc == 8)
    {
        if (g_GameManager.sides[side].stats->lives >= 10.0f)
        {
            this->unknown11e9c = 99999990;
        }
        else
        {
            this->unknown11e9c = (int)g_GameManager.sides[side].stats->lives * 10000000;
        }
    }
    else
    {
        this->unknown11e9c = 0;
    }
    this->bonusTotal = this->unknown11e88 + this->unknown11e8c + this->unknown11e90 + this->unknown11e94 +
                       this->unknown11e98 + this->unknown11e9c;
    if ((g_GameManager.IsMode0() || g_GameManager.IsMode1()) && side == 0)
    {
        g_PlayRecord.OpenEntry(g_GameManager.sides[1].character);
        if (g_GameManager.unknownFc == 8)
        {
            this->anm->StartScriptAtOrigin(&this->unknown10968[4], 0x40);
            g_GameManager.sides[0].stats->extendsGiven = 9999;
        }
        if (g_GameManager.config->unknownAc > 0 && g_GameManager.IsMode0())
        {
            this->anm->StartScriptAtOrigin(&this->unknown10968[5], 0x41);
            this->bonusTotal = 0;
        }
    }
    g_GameManager.sides[this->unknown10964].AddScore(this->bonusTotal);
}

void Gui::UpdateEnemyMarker(int side, D3DXVECTOR3 *pos)
{
    float dist;

    g_App.SetCamera(side);
    if (!(pos->x < -112.0f || pos->x > 112.0f))
    {
        dist = g_GameManager.sides[side].player->pos.x - pos->x;
        if (dist < 0.0f)
        {
            dist = -dist;
        }
        if (dist < 64.0f)
        {
            this->enemyMarkers[side].alpha = (int)dist * 3 + 0x40;
        }
        else
        {
            this->enemyMarkers[side].alpha = 0xff;
        }
        this->enemyMarkers[side].pos.x = g_GameManager.ArenaToScreenX(pos->x);
        this->enemyMarkers[side].pos.y = 472.0f;
        this->enemyMarkers[side].pos.z = 0.0f;
        return;
    }
    this->enemyMarkers[side].alpha = 0;
}

void GuiMsg::Clear()
{
    memset(this, 0, sizeof(GuiMsg));
    this->scriptIndex = -1;
}

WORD PlayerInput::IsHeld2(WORD mask)
{
    return this->unknown2c & mask;
}

WORD PlayerInput::IsJustPressed2(WORD mask)
{
    return this->unknown32 & mask;
}

int FreesResources()
{
    return g_App.freesResources;
}

BOOL GameManager::IsMode2()
{
    return this->mode == 2;
}

void GuiSide::StartUnknownAbcc()
{
    this->unknownAbcc = 1;
    this->timerAbd0 = 0;
}

void Gui::Reset()
{
    if (g_GameManager.IsMode0() || g_GameManager.IsMode1())
    {
        this->msg.Start(g_GameManager.unknown100 == 0 ? g_GameManager.unknownF0->unknown6 : 201, 0);
    }
    else
    {
        this->msg.Start(g_GameManager.unknown100 == 0 ? g_GameManager.unknownF0->unknown6 : 201, 0);
    }
    this->unknown1095c = 0;
    this->unknown10960 = 0;
    this->unknown10964 = 0;
    this->unknown11ea8 = 0;
}

int Gui::FreeResources(Gui *gui)
{
    if (FreesResources())
    {
        gui->msg.UnloadBgm(g_GameManager.sides[1].character * 10);
    }
    return 0;
}

void Gui::RemoveChain(Gui *gui)
{
    if (gui != NULL)
    {
        FreeResources(gui);
        g_Chain.Remove(gui->drawElem);
        g_Chain.Remove(gui->calcElem);
        delete gui;
    }
}

int GuiMsg::Load(char *path, char *path2, int firstScript)
{
    int i;
    BYTE loaded[0x100];

    this->UnloadBgm(firstScript);
    this->Clear();
    if (g_MsgFile == NULL)
    {
        g_MsgFile = this->msgFile = (MsgFile *)OpenPath(path, NULL, 0);
        if (this->msgFile == NULL)
        {
            LogPrintf(&g_Log, "error : メッセージファイル %s が読み込めませんでした\r\n", path);
            return -1;
        }
        for (i = 0; i < this->msgFile->unknown0; i++)
        {
            if (this->msgFile->scripts[i].script != NULL)
            {
                this->msgFile->scripts[i].script = (MsgInstr *)((BYTE *)this->msgFile->scripts[i].script + (int)this->msgFile);
            }
        }
    }
    else
    {
        this->msgFile = g_MsgFile;
    }
    if (!g_GameManager.IsMode2())
    {
        memset(loaded, 0, sizeof(loaded));
        for (i = firstScript; i < firstScript + 10; i++)
        {
            if (i >= this->msgFile->unknown0)
            {
                break;
            }
            this->instr = this->msgFile->scripts[i].script;
            if (this->instr == NULL)
            {
                continue;
            }
            while (this->instr->opcode != 0)
            {
                if (this->instr->opcode == 7 && !loaded[this->instr->i])
                {
                    g_App.LoadBgm(this->instr->i);
                    loaded[this->instr->i] = 1;
                }
                this->instr = (MsgInstr *)((BYTE *)this->instr + this->instr->argSize + 4);
            }
        }
        return 0;
    }
    if (g_MsgFile2 == NULL)
    {
        g_MsgFile2 = this->msgFile2 = (MsgFile *)OpenPath(path2, NULL, 0);
        if (this->msgFile2 == NULL)
        {
            LogPrintf(&g_Log, "error : メッセージファイル %s が読み込めませんでした\r\n", path2);
            return -1;
        }
        for (i = 0; i < this->msgFile2->unknown0; i++)
        {
            this->msgFile2->scripts[i].script = (MsgInstr *)((BYTE *)this->msgFile2->scripts[i].script + (int)this->msgFile2);
        }
    }
    else
    {
        this->msgFile2 = g_MsgFile2;
    }
    g_App.LoadBgm(g_GameManager.unknownF0->unknownC);
    return 0;
}

int Gui::AddedCallback(Gui *gui)
{
    int i;
    int j;
    SpriteEngine *se = g_SpriteEngine;
    AnmLoaded *anm = se->GetAnm(10);

    if (anm->file == NULL)
    {
        gui->anm = se->PreloadAnm(10, "front.anm");
    }
    else
    {
        gui->anm = anm;
    }
    if (gui->anm == NULL)
    {
        return -1;
    }
    gui->anm->StartScriptAtOrigin(&gui->unknownC[0], 0);
    gui->anm->StartScriptAtOrigin(&gui->unknownC[1], 1);
    gui->anm->StartScriptAtOrigin(&gui->unknownC[2], 2);
    gui->anm->StartScriptAtOrigin(&gui->unknownC[3], 3);
    gui->anm->StartScriptAtOrigin(&gui->unknownC[4], 4);
    gui->anm->StartScriptAtOrigin(&gui->unknownC[5], 5);
    gui->anm->StartScriptAtOrigin(&gui->unknownC[6], 6);
    g_AsciiText.anm->StartScriptAtOrigin(&gui->enemyMarkers[0], 0);
    g_AsciiText.anm->StartScriptAtOrigin(&gui->enemyMarkers[1], 0);
    gui->enemyMarkers[0].alpha = 0;
    gui->enemyMarkers[1].alpha = 0;
    if (g_GameManager.IsMode0() || g_GameManager.IsMode1())
    {
        gui->anm->StartScriptAtOrigin(&gui->unknown11eac[0], 0x55);
        gui->anm->StartScriptAtOrigin(&gui->unknown11eac[1], 0x56);
        gui->anm->SetSprite(&gui->unknown11eac[1], g_GameManager.unknownFc + 0x49);
    }
    for (i = 0; i < 2; i++)
    {
        for (j = 0; j < 5; j++)
        {
            gui->unknownE86c[i][j].w = 1.0f;
            gui->unknownE86c[i][j].diffuse = 0x80ffffff;
        }
    }
    gui->unknownE86c[0][0].pos.x = 15.0f;
    gui->unknownE86c[0][0].pos.y = 15.0f;
    gui->unknownE86c[0][0].pos.z = 0.0f;
    gui->unknownE86c[0][1].pos.x = 15.0f;
    gui->unknownE86c[0][1].pos.y = 464.0f;
    gui->unknownE86c[0][1].pos.z = 0.0f;
    gui->unknownE86c[0][2].pos.x = 304.0f;
    gui->unknownE86c[0][2].pos.y = 464.0f;
    gui->unknownE86c[0][2].pos.z = 0.0f;
    gui->unknownE86c[0][3].pos.x = 304.0f;
    gui->unknownE86c[0][3].pos.y = 15.0f;
    gui->unknownE86c[0][3].pos.z = 0.0f;
    gui->unknownE86c[0][4].pos = gui->unknownE86c[0][0].pos;
    gui->unknownE86c[1][0].pos.x = 335.0f;
    gui->unknownE86c[1][0].pos.y = 15.0f;
    gui->unknownE86c[1][0].pos.z = 0.0f;
    gui->unknownE86c[1][1].pos.x = 335.0f;
    gui->unknownE86c[1][1].pos.y = 464.0f;
    gui->unknownE86c[1][1].pos.z = 0.0f;
    gui->unknownE86c[1][2].pos.x = 624.0f;
    gui->unknownE86c[1][2].pos.y = 464.0f;
    gui->unknownE86c[1][2].pos.z = 0.0f;
    gui->unknownE86c[1][3].pos.x = 624.0f;
    gui->unknownE86c[1][3].pos.y = 15.0f;
    gui->unknownE86c[1][3].pos.z = 0.0f;
    gui->unknownE86c[1][4].pos = gui->unknownE86c[1][0].pos;
    gui->msg.Clear();
    if (g_GameManager.IsMode0() || g_GameManager.IsMode1())
    {
        if (gui->msg.Load(g_CharacterFiles[g_GameManager.sides[0].character][3], NULL,
                          g_GameManager.sides[1].character * 10) != 0)
        {
            return -1;
        }
    }
    else
    {
        if (gui->msg.Load(g_CharacterFiles[g_GameManager.sides[0].character][4],
                          g_CharacterFiles[g_GameManager.sides[1].character][4], 0) != 0)
        {
            return -1;
        }
    }
    gui->Reset();
    return 0;
}

Gui *Gui::RegisterChain()
{
    Gui *gui = (Gui *)g_MemoryManager.Register(new Gui(), sizeof(Gui), "FrontInf");

    gui->Init();
    gui->calcElem = g_Chain.CreateElem((ChainCallback)Gui::OnUpdate);
    gui->calcElem->arg = gui;
    gui->calcElem->addedCallback = (ChainCallback)Gui::AddedCallback;
    if (g_Chain.AddToCalcChain(gui->calcElem, 0x16) != 0)
    {
        return NULL;
    }
    gui->drawElem = g_Chain.CreateElem((ChainCallback)Gui::OnDraw);
    gui->drawElem->arg = gui;
    g_Chain.AddToDrawChain(gui->drawElem, 0x1c);
    return gui;
}

// Unloads ANM slot 7 if the side's front has unknownA678.unknown4 set. Returns 0.
// Static, with the side in eax (0x418930).
static int FreeResources(GuiSide *side)
{
    if (side->unknownA678.unknown4 != NULL)
    {
        g_SpriteEngine->ReleaseAnm(7);
    }
    return 0;
}

void GuiSide::SetUnknown76d4Time(int frames)
{
    int secs;
    int hundredths;

    if (frames < 0)
    {
        memset(&this->unknown76d4[0], 0, sizeof(this->unknown76d4[0]));
        memset(&this->unknown76d4[1], 0, sizeof(this->unknown76d4[1]));
        memset(&this->unknown76d4[2], 0, sizeof(this->unknown76d4[2]));
        memset(&this->unknown76d4[3], 0, sizeof(this->unknown76d4[3]));
        memset(&this->unknown76d4[4], 0, sizeof(this->unknown76d4[4]));
        memset(&this->unknown76d4[5], 0, sizeof(this->unknown76d4[5]));
        return;
    }
    secs = frames / 60;
    if (secs / 100 == 0)
    {
        memset(&this->unknown76d4[0], 0, sizeof(this->unknown76d4[0]));
    }
    else
    {
        this->anm->SetSprite(&this->unknown76d4[0], secs / 100 + 0x35);
    }
    this->anm->SetSprite(&this->unknown76d4[1], secs / 10 % 10 + 0x35);
    this->anm->SetSprite(&this->unknown76d4[2], secs % 10 + 0x35);
    hundredths = frames % 60 * 100 / 60;
    this->anm->SetSprite(&this->unknown76d4[4], hundredths / 10 + 0x35);
    this->anm->SetSprite(&this->unknown76d4[5], hundredths % 10 + 0x35);
}

int GuiSide::OnUpdate(GuiSide *side)
{
    int i;
    int n;
    Sprite *sprite;
    GameSide *gameSide;
    Player *player;

    g_App.SetCamera(side->sideIdx);
    for (i = 0; i < ARRAY_SIZE(side->unknown0); i++)
    {
        g_SpriteEngine->Step(&side->unknown0[i]);
    }
    n = side->side->player->unknown30410.count;
    if (n > 999)
    {
        n = 999;
    }
    if (n >= 100)
    {
        side->anm->SetSprite(&side->unknown0[2], n / 100 + 8);
        side->unknown0[2].visible = 1;
    }
    else
    {
        side->unknown0[2].visible = 0;
    }
    if (n >= 10)
    {
        side->anm->SetSprite(&side->unknown0[3], n / 10 % 10 + 8);
        side->unknown0[3].visible = 1;
    }
    else
    {
        side->unknown0[3].visible = 0;
    }
    side->anm->SetSprite(&side->unknown0[4], n % 10 + 8);
    player = side->side->player;
    if (player->unknown30410.timer20 > 0 && player->unknown30410.timer20 < 20)
    {
        if (player->unknown30410.timer20.AsFrames() & 1)
        {
            for (i = 1; i < 5; i++)
            {
                side->unknown0[i].colorRgb[2] = 0xff;
                side->unknown0[i].colorRgb[1] = 0;
                side->unknown0[i].colorRgb[0] = 0;
            }
        }
        else
        {
            for (i = 1; i < 5; i++)
            {
                side->unknown0[i].colorRgb[2] = 0xff;
                side->unknown0[i].colorRgb[1] = 0xff;
                side->unknown0[i].colorRgb[0] = 0xff;
            }
        }
    }
    else
    {
        for (i = 1; i < 5; i++)
        {
            side->unknown0[i].colorRgb[2] = 0xd0;
            side->unknown0[i].colorRgb[1] = 0x80;
            side->unknown0[i].colorRgb[0] = 0x80;
        }
    }
    n = side->side->player->unknown30410.bonus;
    side->anm->SetSprite(&side->unknown0[5], n / 100000 + 8);
    n %= 100000;
    side->anm->SetSprite(&side->unknown0[6], n / 10000 + 8);
    n %= 10000;
    side->anm->SetSprite(&side->unknown0[7], n / 1000 + 8);
    n %= 1000;
    side->anm->SetSprite(&side->unknown0[8], n / 100 + 8);
    n %= 100;
    side->anm->SetSprite(&side->unknown0[9], n / 10 + 8);
    n %= 10;
    side->anm->SetSprite(&side->unknown0[10], n + 8);
    n = side->side->stats->guiScore;
    if (n >= 100000000)
    {
        side->anm->SetSprite(&side->unknown3f60[0], n / 100000000 + 8);
        n %= 100000000;
        side->unknown3f60[0].visible = 1;
    }
    else
    {
        side->unknown3f60[0].visible = 0;
    }
    side->anm->SetSprite(&side->unknown3f60[1], n / 10000000 + 8);
    n %= 10000000;
    side->anm->SetSprite(&side->unknown3f60[2], n / 1000000 + 8);
    n %= 1000000;
    side->anm->SetSprite(&side->unknown3f60[3], n / 100000 + 8);
    n %= 100000;
    side->anm->SetSprite(&side->unknown3f60[4], n / 10000 + 8);
    n %= 10000;
    side->anm->SetSprite(&side->unknown3f60[5], n / 1000 + 8);
    n %= 1000;
    side->anm->SetSprite(&side->unknown3f60[6], n / 100 + 8);
    n %= 100;
    side->anm->SetSprite(&side->unknown3f60[7], n / 10 + 8);
    n %= 10;
    side->anm->SetSprite(&side->unknown3f60[8], n + 8);
    side->anm->SetSprite(&side->unknown3f60[9], side->side->stats->unknown14 + 8);
    g_SpriteEngine->StepSprites(side->unknown3f60, 10);
    n = side->side->player->unknownA8;
    for (i = 0; i < ARRAY_SIZE(side->unknown1d0c); i++)
    {
        sprite = &side->unknown1d0c[i];
        g_SpriteEngine->Step(sprite);
        side->anm->SetSprite(sprite, (n >= 2 ? 2 : n) + 0x23);
        n -= 2;
        if (n < 0)
        {
            n = 0;
        }
    }
    for (i = 0; i < ARRAY_SIZE(side->unknown2a40); i++)
    {
        g_SpriteEngine->Step(&side->unknown2a40[i]);
    }
    gameSide = side->side;
    side->unknown2a40[0].scaleY = gameSide->player->unknown30388 * 0.0025f;
    side->unknown2a40[1].scaleY = gameSide->player->unknown30384 * 0.0025f;
    if (gameSide->player->unknown30384 >= 400.0f)
    {
        side->unknown2a40[1].color = side->timerAbdc % 2 ? 0xffffffff : 0xffff0000;
    }
    else if (gameSide->player->unknown30384 >= 300.0f)
    {
        side->unknown2a40[1].color = side->timerAbdc % 2 ? 0xfff0f000 : 0xfff0f0c0;
    }
    else if (gameSide->player->unknown30384 >= 200.0f)
    {
        side->unknown2a40[1].color = side->timerAbdc % 2 ? 0xfff0f000 : 0xffc0c0c0;
    }
    else if (gameSide->player->unknown30384 >= 100.0f)
    {
        side->unknown2a40[1].color = side->timerAbdc % 2 ? 0xfff0f080 : 0xffc0c0c0;
    }
    else
    {
        side->unknown2a40[1].color = 0xffc0c0c0;
    }
    if (gameSide->player->unknown30388 >= 400.0f)
    {
        side->unknown2a40[0].color = 0xffc08080;
    }
    else if (gameSide->player->unknown30388 >= 300.0f)
    {
        side->unknown2a40[0].color = 0xffb08080;
    }
    else if (gameSide->player->unknown30388 >= 200.0f)
    {
        side->unknown2a40[0].color = 0xffa08080;
    }
    else
    {
        side->unknown2a40[0].color = 0xff808080;
    }
    n = gameSide->player->unknownA0;
    side->anm->SetSprite(&side->unknown2a40[3], n / 10 + 8);
    side->anm->SetSprite(&side->unknown2a40[4], n % 10 + 8);
    n = side->side->player->unknownA4;
    side->anm->SetSprite(&side->unknown2a40[5], n / 10 + 8);
    side->anm->SetSprite(&side->unknown2a40[6], n % 10 + 8);
    g_SpriteEngine->Step(&side->unknownA678.sprites[0]);
    g_SpriteEngine->Step(&side->unknownA678.sprites[1]);
    side->unknown3cbc.scaleY = side->side->player->unknown30410.timer14.AsFloat() * (1.0f / 105.0f);
    g_SpriteEngine->Step(&side->unknown3cbc);
    g_SpriteEngine->Step(&side->unknown59c8);
    g_SpriteEngine->Step(&side->unknown5c6c[0]);
    sprite = &side->unknown5c6c[1];
    side->anm->SetSprite(sprite, (int)side->side->player->unknown30384 / 100 + 0x35);
    g_SpriteEngine->Step(sprite);
    sprite = &side->unknown5c6c[2];
    side->anm->SetSprite(sprite, (int)side->side->player->unknown30388 / 100 + 0x35);
    g_SpriteEngine->Step(sprite);
    g_SpriteEngine->Step(&side->unknown5c6c[3]);
    g_SpriteEngine->StepSprites(side->unknown66fc, 3);
    g_SpriteEngine->StepSprites(side->unknown6ee8, 2);
    g_SpriteEngine->StepSprites(side->unknown76d4, 6);
    g_SpriteEngine->Step(&side->unknown7430);
    if (!g_GameManager.IsMode2())
    {
        for (i = 0; i < (int)g_GameManager.sides[side->sideIdx].stats->lives; i++)
        {
            side->unknown86ac[i].visible = 1;
        }
        for (; i < ARRAY_SIZE(side->unknown86ac); i++)
        {
            side->unknown86ac[i].visible = 0;
        }
        g_SpriteEngine->StepSprites(side->unknown86ac, 7);
    }
    n = g_GameManager.unknown108[side->sideIdx];
    if (n > 5)
    {
        n = 5;
    }
    for (i = 0; i < n; i++)
    {
        side->unknown9928[i].visible = 1;
    }
    for (; i < ARRAY_SIZE(side->unknown9928); i++)
    {
        side->unknown9928[i].visible = 0;
    }
    g_SpriteEngine->StepSprites(side->unknown9928, 5);
    if (side->unknownAbcc == 1)
    {
        side->timerAbd0++;
    }
    else if (side->unknownAbcc == 2)
    {
        side->timerAbd0--;
        if (side->timerAbd0 <= 0)
        {
            side->unknownAbcc = 0;
        }
    }
    side->timerAbdc++;
    return 1;
}

int GuiSide::OnDraw(GuiSide *side)
{
    int i;
    GameSide *gameSide;

    g_App.cameras[side->sideIdx].shakeX = 0.0f;
    g_App.cameras[side->sideIdx].shakeY = 0.0f;
    g_App.SetView2D(side->sideIdx);
    for (i = 0; i < ARRAY_SIZE(side->unknown0); i++)
    {
        side->unknown0[i].pos.x = g_GameManager.ArenaToScreenX(side->unknown0[i].posOffset.x);
        side->unknown0[i].pos.y = g_GameManager.ArenaToScreenY(side->unknown0[i].posOffset.y);
        side->unknown0[i].pos.z = 0.0f;
        g_SpriteEngine->Draw(&side->unknown0[i]);
    }
    for (i = 0; i < ARRAY_SIZE(side->unknown1d0c); i++)
    {
        side->unknown1d0c[i].pos.x = g_GameManager.ArenaToScreenX(side->unknown1d0c[i].posOffset.x);
        side->unknown1d0c[i].pos.y = g_GameManager.ArenaToScreenY(side->unknown1d0c[i].posOffset.y);
        side->unknown1d0c[i].pos.z = 0.0f;
        g_SpriteEngine->Draw(&side->unknown1d0c[i]);
    }
    for (i = 0; i < ARRAY_SIZE(side->unknown2a40); i++)
    {
        side->unknown2a40[i].pos.x = g_GameManager.ArenaToScreenX(side->unknown2a40[i].posOffset.x);
        side->unknown2a40[i].pos.y = g_GameManager.ArenaToScreenY(side->unknown2a40[i].posOffset.y);
        side->unknown2a40[i].pos.z = 0.0f;
        g_SpriteEngine->Draw(&side->unknown2a40[i]);
    }
    for (i = 0; i < ARRAY_SIZE(side->unknownA678.sprites); i++)
    {
        side->unknownA678.sprites[i].pos.x = g_GameManager.ArenaToScreenX(
            side->unknownA678.sprites[i].posOffset.x);
        side->unknownA678.sprites[i].pos.y = g_GameManager.ArenaToScreenY(
            side->unknownA678.sprites[i].posOffset.y);
        side->unknownA678.sprites[i].pos.z = 0.0f;
        g_SpriteEngine->DrawUnrotated(&side->unknownA678.sprites[i]);
    }
    side->unknown3cbc.pos.x = g_GameManager.ArenaToScreenX(side->unknown3cbc.posOffset.x);
    side->unknown3cbc.pos.y = g_GameManager.ArenaToScreenY(side->unknown3cbc.posOffset.y);
    side->unknown3cbc.pos.z = 0.0f;
    g_SpriteEngine->DrawUnrotated(&side->unknown3cbc);
    for (i = 0; i < ARRAY_SIZE(side->unknown3f60); i++)
    {
        side->unknown3f60[i].pos.x = g_GameManager.ArenaToScreenX(side->unknown3f60[i].posOffset.x);
        side->unknown3f60[i].pos.y = g_GameManager.ArenaToScreenY(side->unknown3f60[i].posOffset.y);
        side->unknown3f60[i].pos.z = 0.0f;
        g_SpriteEngine->DrawUnrotated(&side->unknown3f60[i]);
    }
    side->unknown59c8.pos.x = g_GameManager.ArenaToScreenX(side->unknown59c8.posOffset.x);
    side->unknown59c8.pos.y = g_GameManager.ArenaToScreenY(side->unknown59c8.posOffset.y);
    g_SpriteEngine->DrawUnrotated(&side->unknown59c8);
    for (i = 0; i < ARRAY_SIZE(side->unknown5c6c); i++)
    {
        side->unknown5c6c[i].pos.x = g_GameManager.ArenaToScreenX(
            side->side->player->pos.x + side->unknown5c6c[i].posOffset.x);
        side->unknown5c6c[i].pos.y = g_GameManager.ArenaToScreenY(
            side->side->player->pos.y + side->unknown5c6c[i].posOffset.y);
        g_SpriteEngine->DrawUnrotated(&side->unknown5c6c[i]);
    }
    for (i = 0; i < ARRAY_SIZE(side->unknown66fc); i++)
    {
        side->unknown66fc[i].pos.x = g_GameManager.ArenaToScreenX(
            side->side->player->pos.x + side->unknown66fc[i].posOffset.x);
        side->unknown66fc[i].pos.y = g_GameManager.ArenaToScreenY(
            side->side->player->pos.y + side->unknown66fc[i].posOffset.y);
        if (side->unknownA65c != 0)
        {
            side->unknown66fc[i].pos.y -= 24.0f;
        }
        g_SpriteEngine->DrawUnrotated(&side->unknown66fc[i]);
    }
    for (i = 0; i < ARRAY_SIZE(side->unknown76d4); i++)
    {
        side->unknown76d4[i].pos.x = g_GameManager.ArenaToScreenX(
            side->side->player->pos.x + side->unknown76d4[i].posOffset.x);
        side->unknown76d4[i].pos.y = g_GameManager.ArenaToScreenY(
            side->side->player->pos.y + side->unknown76d4[i].posOffset.y);
        g_SpriteEngine->DrawUnrotated(&side->unknown76d4[i]);
    }
    for (i = 0; i < ARRAY_SIZE(side->unknown6ee8); i++)
    {
        side->unknown6ee8[i].pos.x = g_GameManager.ArenaToScreenX(
            side->side->player->pos.x + side->unknown6ee8[i].posOffset.x);
        side->unknown6ee8[i].pos.y = g_GameManager.ArenaToScreenY(
            side->side->player->pos.y + side->unknown6ee8[i].posOffset.y);
        g_SpriteEngine->DrawUnrotated(&side->unknown6ee8[i]);
    }
    gameSide = side->side;
    side->unknown7430.pos.x = g_GameManager.ArenaToScreenX(
        gameSide->player->pos.x + side->unknown7430.posOffset.x);
    side->unknown7430.pos.y = g_GameManager.ArenaToScreenY(
        gameSide->player->pos.y + side->unknown7430.posOffset.y);
    g_SpriteEngine->DrawUnrotated(&side->unknown7430);
    for (i = 0; i < ARRAY_SIZE(side->unknown86ac); i++)
    {
        side->unknown86ac[i].pos.x = g_GameManager.ArenaToScreenX(side->unknown86ac[i].posOffset.x);
        side->unknown86ac[i].pos.y = g_GameManager.ArenaToScreenY(side->unknown86ac[i].posOffset.y);
        g_SpriteEngine->DrawUnrotated(&side->unknown86ac[i]);
    }
    for (i = 0; i < ARRAY_SIZE(side->unknown9928); i++)
    {
        side->unknown9928[i].pos.x = g_GameManager.ArenaToScreenX(side->unknown9928[i].posOffset.x);
        side->unknown9928[i].pos.y = g_GameManager.ArenaToScreenY(side->unknown9928[i].posOffset.y);
        g_SpriteEngine->Draw(&side->unknown9928[i]);
    }
    if (side->unknownAbcc != 0)
    {
        GuiUnknownE86c vertices[4];
        float t;
        float dir;
        float x;

        if (side->timerAbd0.AsFloat() >= 30.0f)
        {
            t = 30.0f;
        }
        else
        {
            t = side->timerAbd0.AsFloat();
        }
        dir = 1.0f;
        if (side->unknownAbcc != 1)
        {
            dir = -1.0f;
        }
        g_SpriteEngine->FlushQuadBatch();
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
        g_App.SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        g_App.d3dDevice->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
        t *= 448.0f / 30.0f;
        x = dir * -144.0f;
        vertices[0].pos = D3DXVECTOR3(g_GameManager.ArenaToScreenX(x),
                                      g_GameManager.ArenaToScreenY(0.0f), 0.0f);
        dir *= 144.0f;
        vertices[1].pos = D3DXVECTOR3(g_GameManager.ArenaToScreenX(dir),
                                      g_GameManager.ArenaToScreenY(0.0f), 0.0f);
        vertices[2].pos = D3DXVECTOR3(g_GameManager.ArenaToScreenX(x), g_GameManager.ArenaToScreenY(t), 0.0f);
        vertices[0].diffuse = vertices[1].diffuse = vertices[2].diffuse = 0xff000000;
        vertices[0].w = vertices[1].w = vertices[2].w = 1.0f;
        g_App.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 1, vertices, sizeof(GuiUnknownE86c));
        t = 448.0f - side->timerAbd0.AsFloat() * (448.0f / 30.0f);
        vertices[0].pos = D3DXVECTOR3(g_GameManager.ArenaToScreenX(x),
                                      g_GameManager.ArenaToScreenY(448.0f), 0.0f);
        vertices[1].pos = D3DXVECTOR3(g_GameManager.ArenaToScreenX(dir),
                                      g_GameManager.ArenaToScreenY(448.0f), 0.0f);
        vertices[2].pos = D3DXVECTOR3(g_GameManager.ArenaToScreenX(dir),
                                      g_GameManager.ArenaToScreenY(t), 0.0f);
        vertices[0].diffuse = vertices[1].diffuse = vertices[2].diffuse = 0xff000000;
        vertices[0].w = vertices[1].w = vertices[2].w = 1.0f;
        g_App.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 1, vertices, sizeof(GuiUnknownE86c));
        t = side->timerAbd0.AsFloat() * (288.0f / 30.0f) - 144.0f;
        if (side->unknownAbcc == 1)
        {
            vertices[0].pos = D3DXVECTOR3(g_GameManager.ArenaToScreenX(-144.0f),
                                          g_GameManager.ArenaToScreenY(0.0f), 0.0f);
            vertices[1].pos = D3DXVECTOR3(g_GameManager.ArenaToScreenX(-144.0f),
                                          g_GameManager.ArenaToScreenY(448.0f), 0.0f);
            vertices[2].pos = D3DXVECTOR3(g_GameManager.ArenaToScreenX(t),
                                          g_GameManager.ArenaToScreenY(448.0f), 0.0f);
        }
        else
        {
            vertices[0].pos = D3DXVECTOR3(g_GameManager.ArenaToScreenX(-144.0f),
                                          g_GameManager.ArenaToScreenY(448.0f), 0.0f);
            vertices[1].pos = D3DXVECTOR3(g_GameManager.ArenaToScreenX(-144.0f),
                                          g_GameManager.ArenaToScreenY(0.0f), 0.0f);
            vertices[2].pos = D3DXVECTOR3(g_GameManager.ArenaToScreenX(t),
                                          g_GameManager.ArenaToScreenY(0.0f), 0.0f);
        }
        vertices[0].diffuse = vertices[1].diffuse = vertices[2].diffuse = 0xff000000;
        vertices[0].w = vertices[1].w = vertices[2].w = 1.0f;
        g_App.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 1, vertices, sizeof(GuiUnknownE86c));
        t = 144.0f - side->timerAbd0.AsFloat() * (288.0f / 30.0f);
        if (side->unknownAbcc == 1)
        {
            vertices[0].pos = D3DXVECTOR3(g_GameManager.ArenaToScreenX(144.0f),
                                          g_GameManager.ArenaToScreenY(448.0f), 0.0f);
            vertices[1].pos = D3DXVECTOR3(g_GameManager.ArenaToScreenX(144.0f),
                                          g_GameManager.ArenaToScreenY(0.0f), 0.0f);
            vertices[2].pos = D3DXVECTOR3(g_GameManager.ArenaToScreenX(t),
                                          g_GameManager.ArenaToScreenY(0.0f), 0.0f);
        }
        else
        {
            vertices[0].pos = D3DXVECTOR3(g_GameManager.ArenaToScreenX(144.0f),
                                          g_GameManager.ArenaToScreenY(0.0f), 0.0f);
            vertices[1].pos = D3DXVECTOR3(g_GameManager.ArenaToScreenX(144.0f),
                                          g_GameManager.ArenaToScreenY(448.0f), 0.0f);
            vertices[2].pos = D3DXVECTOR3(g_GameManager.ArenaToScreenX(t),
                                          g_GameManager.ArenaToScreenY(448.0f), 0.0f);
        }
        vertices[0].diffuse = vertices[1].diffuse = vertices[2].diffuse = 0xff000000;
        vertices[0].w = vertices[1].w = vertices[2].w = 1.0f;
        g_App.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 1, vertices, sizeof(GuiUnknownE86c));
        g_SpriteEngine->ClearVertexFormat();
        g_SpriteEngine->ClearColorOp();
        g_SpriteEngine->ClearBlendMode();
        g_SpriteEngine->ClearZWrite();
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    }
    return 1;
}

int GuiSide::AddedCallback(GuiSide *side)
{
    DWORD i;
    int sideIdx;

    side->anm = g_SpriteEngine->GetAnm(10);
    if (side->anm == NULL)
    {
        return -1;
    }
    sideIdx = side->sideIdx;
    side->unknownA678.anm = g_SpriteEngine->GetAnm(sideIdx + 5);
    if (sideIdx == 0 && g_GameManager.unknownFc == 8 && side->side->character == 6)
    {
        side->unknownA678.unknown4 =
            g_SpriteEngine->PreloadAnm(7, g_Pl06FcAnmFiles[g_GameManager.sides[0].unknown30]);
        if (side->unknownA678.unknown4 == NULL)
        {
            return -1;
        }
    }
    for (i = 0; i < ARRAY_SIZE(side->unknown0); i++)
    {
        side->anm->StartScriptAtOrigin(&side->unknown0[i], i + 7);
        side->unknown0[i].posOffset = side->unknown0[i].pos;
    }
    for (i = 0; i < ARRAY_SIZE(side->unknown1d0c); i++)
    {
        side->anm->StartScriptAtOrigin(&side->unknown1d0c[i], i + 0x23);
    }
    for (i = 0; i < ARRAY_SIZE(side->unknown2a40); i++)
    {
        side->anm->StartScriptAtOrigin(&side->unknown2a40[i], i + 0x34);
    }
    for (i = 0; i < ARRAY_SIZE(side->unknown3f60); i++)
    {
        side->anm->StartScriptAtOrigin(&side->unknown3f60[i], i + 0x14);
    }
    side->anm->StartScriptAtOrigin(&side->unknown3cbc, 0x3b);
    if (!g_GameManager.IsMode2())
    {
        for (i = 0; i < ARRAY_SIZE(side->unknown86ac); i++)
        {
            side->anm->StartScriptAtOrigin(&side->unknown86ac[i], i + 0x28);
        }
    }
    for (i = 0; i < ARRAY_SIZE(side->unknown9928); i++)
    {
        side->anm->StartScriptAtOrigin(&side->unknown9928[i], i + 0x2f);
    }
    side->timerAbdc = 0;
    memset(&side->unknown59c8, 0, sizeof(side->unknown59c8));
    return 0;
}

void GuiSide::RemoveChain(GuiSide *side)
{
    if (side != NULL)
    {
        FreeResources(side);
        g_Chain.Remove(side->drawElem);
        g_Chain.Remove(side->calcElem);
        delete side;
    }
}

void GuiSide::StartUnknown59c8()
{
    this->anm->StartScriptAtOrigin(&this->unknown59c8, 0x43);
}

void GuiSide::Reset()
{
    this->timerAbdc = 0;
    if (this->unknownAbcc != 0)
    {
        this->unknownAbcc = 2;
        this->timerAbd0 = 30;
    }
}

void GuiSide::StartUnknown5c6c()
{
    this->anm->StartScriptAtOrigin(&this->unknown5c6c[0], 0x44);
    this->anm->StartScriptAtOrigin(&this->unknown5c6c[1], 0x45);
    this->anm->StartScriptAtOrigin(&this->unknown5c6c[2], 0x46);
    this->anm->StartScriptAtOrigin(&this->unknown5c6c[3], 0x47);
    this->unknownA65c = 1;
}

void GuiSide::StopUnknown5c6c()
{
    this->unknown5c6c[0].SetPendingInterrupt(1);
    this->unknown5c6c[1].SetPendingInterrupt(1);
    this->unknown5c6c[2].SetPendingInterrupt(1);
    this->unknown5c6c[3].SetPendingInterrupt(1);
    this->unknownA65c = 0;
}

void GuiSide::StartUnknown66fc(int digit)
{
    this->anm->StartScriptAtOrigin(&this->unknown66fc[0], 0x48);
    this->anm->StartScriptAtOrigin(&this->unknown66fc[1], 0x49);
    this->anm->StartScriptAtOrigin(&this->unknown66fc[2], 0x4a);
    this->anm->SetSprite(&this->unknown66fc[2], digit + 0x35);
}

void GuiSide::StartUnknown6ee8(int idx)
{
    this->anm->StartScriptAtOrigin(&this->unknown6ee8[0], 0x4b);
    this->anm->StartScriptAtOrigin(&this->unknown6ee8[1], idx + 0x4c);
}

void GuiSide::StartUnknown7430()
{
    this->anm->StartScriptAtOrigin(&this->unknown7430, 0x4e);
}

void GuiSide::StartUnknown76d4(int frames)
{
    this->anm->StartScriptAtOrigin(&this->unknown76d4[0], 0x4f);
    this->anm->StartScriptAtOrigin(&this->unknown76d4[1], 0x50);
    this->anm->StartScriptAtOrigin(&this->unknown76d4[2], 0x51);
    this->anm->StartScriptAtOrigin(&this->unknown76d4[3], 0x52);
    this->anm->StartScriptAtOrigin(&this->unknown76d4[4], 0x53);
    this->anm->StartScriptAtOrigin(&this->unknown76d4[5], 0x54);
    this->SetUnknown76d4Time(frames);
}

GuiSide *GuiSide::RegisterChain(int sideIdx)
{
    GuiSide *side = (GuiSide *)g_MemoryManager.Register(new GuiSide(), sizeof(GuiSide), "FrontSideInf");

    side->Init();
    side->sideIdx = sideIdx;
    side->side = &g_GameManager.sides[sideIdx];
    side->otherSide = &g_GameManager.sides[1 - sideIdx];
    side->calcElem = g_Chain.CreateElem((ChainCallback)GuiSide::OnUpdate);
    side->calcElem->arg = side;
    side->calcElem->addedCallback = (ChainCallback)GuiSide::AddedCallback;
    if (g_Chain.AddToCalcChain(side->calcElem, sideIdx + 0x17) != 0)
    {
        return NULL;
    }
    side->drawElem = g_Chain.CreateElem((ChainCallback)GuiSide::OnDraw);
    side->drawElem->arg = side;
    g_Chain.AddToDrawChain(side->drawElem, sideIdx + 0x1a);
    return side;
}
