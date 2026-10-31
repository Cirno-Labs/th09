#include "gui.h"
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
    this->unknown1d6d = 1;
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
    if (side->unknownA678.unknown4 != 0)
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
