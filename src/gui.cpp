#include "gui.h"
#include "ascii.h"
#include "gamemanager.h"
#include "player.h"
#include "sprite.h"

int Gui::LoadAnm()
{
    return g_SpriteEngine->PreloadAnm(10, "front.anm") == NULL;
}

void Gui::ReleaseAnm()
{
    g_SpriteEngine->ReleaseAnm(10);
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
    if (this->timer < 40)
    {
        height = this->timer.AsFloat() * 1.5f;
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
            while (((MsgInstr *)this->instr)->opcode != 0)
            {
                if (((MsgInstr *)this->instr)->opcode == 7 && !unloaded[((MsgInstr *)this->instr)->args[0]])
                {
                    g_App.UnloadBgm(((MsgInstr *)this->instr)->args[0]);
                    unloaded[((MsgInstr *)this->instr)->args[0]] = 1;
                }
                this->instr = (BYTE *)this->instr + ((MsgInstr *)this->instr)->argSize + 4;
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
                this->msgFile->scripts[i].script = (BYTE *)this->msgFile->scripts[i].script + (int)this->msgFile;
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
            while (((MsgInstr *)this->instr)->opcode != 0)
            {
                if (((MsgInstr *)this->instr)->opcode == 7 && !loaded[((MsgInstr *)this->instr)->args[0]])
                {
                    g_App.LoadBgm(((MsgInstr *)this->instr)->args[0]);
                    loaded[((MsgInstr *)this->instr)->args[0]] = 1;
                }
                this->instr = (BYTE *)this->instr + ((MsgInstr *)this->instr)->argSize + 4;
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
            this->msgFile2->scripts[i].script = (BYTE *)this->msgFile2->scripts[i].script + (int)this->msgFile2;
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
