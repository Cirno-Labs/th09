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
