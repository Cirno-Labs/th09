#include "gui.h"
#include "gamemanager.h"
#include "sprite.h"

int Gui::LoadAnm()
{
    return g_SpriteEngine->PreloadAnm(10, "front.anm") == NULL;
}

void Gui::ReleaseAnm()
{
    g_SpriteEngine->ReleaseAnm(10);
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
