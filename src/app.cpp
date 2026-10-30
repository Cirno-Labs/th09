#include "timer.h"
#include "unsorted.h"

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
