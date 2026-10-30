#include "timer.h"
#include "unsorted.h"

int App::GetNoFog()
{
    return this->flags.noFog;
}

int App::GetNoScore()
{
    return this->stateFlags >> 3 & 1;
}

int App::GetForce16BitTextures()
{
    return this->flags.force16BitTextures;
}

int App::GetReferenceRasterizer()
{
    return this->flags.referenceRasterizer;
}

int App::GetWindowed()
{
    return this->windowed;
}

int App::GetNoDirectInput()
{
    return this->flags.noDirectInput;
}

int App::GetPreloadBgm()
{
    return this->flags.preloadBgm;
}

void Timer::Add(float frames)
{
    this->previous = this->current;
    if (g_App.gameSpeed > 0.99f)
    {
        this->currentF += frames;
    }
    else
    {
        this->currentF += g_App.gameSpeed * frames;
    }
    this->current = this->currentF;
    if (g_App.stateFlags & 0x20)
    {
        this->previous = -999999;
        this->currentF += this->current;
    }
}

void App::TickTimer(int *frames, float *subFrame)
{
    if (this->gameSpeed <= 0.99f)
    {
        *subFrame += this->gameSpeed;
        *frames = *subFrame;
    }
    else
    {
        (*frames)++;
        *subFrame += 1.0f;
    }
}
