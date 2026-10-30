// The ending, from 0x40e080, as th08's ending.cpp.

#include <stdlib.h>
#include <string.h>

#include "ending.h"
#include "screeneffect.h"
#include "sprite.h"
#include "unsorted.h"

int Ending::ReadInt()
{
    int value;

    value = atoi(this->cursor);
    while (*this->cursor != '\0')
    {
        this->cursor++;
    }
    while (*this->cursor == '\0')
    {
        this->cursor++;
    }
    return value;
}

void Ending::DrawFade()
{
    float rect[4];

    rect[0] = 0.0f;
    rect[1] = 0.0f;
    rect[2] = 640.0f;
    rect[3] = 480.0f;
    if (this->fadeColor & 0xff000000)
    {
        DrawColorRect(rect, this->fadeColor);
    }
}

void Ending::UpdateFade()
{
    int a;

    switch (this->fadeType)
    {
    case 1:
        if (this->fadeTimer >= this->fadeDuration)
        {
            this->fadeType = 0;
            this->fadeColor = 0;
            break;
        }
        a = 0xff - this->fadeTimer * 0xff / this->fadeDuration;
        this->fadeColor = a << 24;
        this->fadeTimer++;
        break;
    case 2:
        if (this->fadeTimer >= this->fadeDuration)
        {
            this->fadeColor = 0xff000000;
            break;
        }
        a = this->fadeTimer * 0xff / this->fadeDuration;
        this->fadeColor = a << 24;
        this->fadeTimer++;
        break;
    case 3:
        if (this->fadeTimer >= this->fadeDuration)
        {
            this->fadeType = 0;
            this->fadeColor = 0;
            break;
        }
        a = 0xff - this->fadeTimer * 0xff / this->fadeDuration;
        this->fadeColor = a << 24 | 0xffffff;
        this->fadeTimer++;
        break;
    case 4:
        if (this->fadeTimer >= this->fadeDuration)
        {
            this->fadeColor = 0xffffffff;
            break;
        }
        a = this->fadeTimer * 0xff / this->fadeDuration;
        this->fadeColor = a << 24 | 0xffffff;
        this->fadeTimer++;
        break;
    case 0:
        this->fadeColor = 0;
        break;
    }
}

int Ending::DeletedCallback(Ending *ending)
{
    g_SpriteEngine->ReleaseAnm(0x13);
    g_App.scene = 6;
    g_SpriteEngine->ReleaseSurface(0);
    g_MemoryManager.Free(ending->file);
    g_Chain.Remove(ending->drawChain);
    ending->drawChain = NULL;
    delete ending;
    g_App.stateFlags &= ~0x400;
    g_App.hideFps = 0;
    return 0;
}

int Ending::OnDraw(Ending *ending)
{
    int i;

    g_SpriteEngine->CopySurfaceRectToBackBuffer(0, 0, 0, ending->scrollX, ending->scrollY, 640, 480);
    for (i = 0; i < 0xf; i++)
    {
        g_SpriteEngine->Draw(&ending->sprites[i]);
    }
    ending->DrawFade();
    return 1;
}

int Ending::Load(char *path)
{
    char *oldFile;

    oldFile = this->file;
    this->file = (char *)OpenPath(path, NULL, 0);
    if (this->file == NULL)
    {
        this->file = oldFile;
        LogPrintf(&g_Log, "error : エンディングファイルが読み込めない、ファイルが破壊されています\r\n");
        return -1;
    }
    this->cursor = this->file;
    this->lineDelay = 8;
    this->lineTimer = 0;
    this->timer = 0;
    if (oldFile != NULL)
    {
        g_MemoryManager.Free(oldFile);
    }
    return 0;
}

Ending::Ending()
{
    memset(this, 0, sizeof(Ending));
    this->lineDelay = 8;
    this->lineTimer = 0;
    this->timer = 0;
    this->scrollX = 0.0f;
    this->scrollY = 0.0f;
    this->scrollSpeed = 0.0f;
}

int Ending::OnUpdate(Ending *ending)
{
    int i;
    int extra = 0;

loop:
    if (ending->RunScript() != 0)
    {
        return 0;
    }
    for (i = 0; i < 0xf; i++)
    {
        g_SpriteEngine->Step(&ending->sprites[i]);
    }
    if (ending->canSkip != 0 && g_PlayerInputs[2].IsHeld(0x100) && extra < 8)
    {
        extra++;
        goto loop;
    }
    return 1;
}

int Ending::RegisterChain()
{
    Ending *ending = (Ending *)g_MemoryManager.Register(new Ending(), sizeof(Ending), "EndingInf");

    ending->calcChain = g_Chain.CreateElem((ChainCallback)Ending::OnUpdate);
    ending->calcChain->arg = ending;
    ending->calcChain->addedCallback = (ChainCallback)Ending::AddedCallback;
    ending->calcChain->deletedCallback = (ChainCallback)Ending::DeletedCallback;
    if (g_Chain.AddToCalcChain(ending->calcChain, 5) != 0)
    {
        return -1;
    }
    ending->drawChain = g_Chain.CreateElem((ChainCallback)Ending::OnDraw);
    ending->drawChain->arg = ending;
    g_Chain.AddToDrawChain(ending->drawChain, 4);
    return 0;
}
