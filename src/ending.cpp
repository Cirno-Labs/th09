// The ending, from 0x40e080, as th08's ending.cpp.

#include "ending.h"
#include "screeneffect.h"
#include "sprite.h"
#include "unsorted.h"

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
