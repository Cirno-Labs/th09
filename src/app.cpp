#include "timer.h"
#include "unsorted.h"

int App::GetNoFog()
{
    return this->flags.noFog;
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
