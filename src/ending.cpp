// The ending, from 0x40e080, as th08's ending.cpp.

#include <stdlib.h>

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
