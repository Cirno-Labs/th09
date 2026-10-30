#include "timer.h"

#include "unsorted.h"

void Timer::Initialize()
{
    this->current = 0;
    this->previous = -999999;
    this->currentF = 0.0f;
}

int Timer::NextTick()
{
    this->previous = this->current;
    g_App.TickTimer(&this->current, &this->currentF);
    return this->current;
}

void Timer::Set(int frames)
{
    this->current = frames;
    this->previous = -999999;
    this->currentF = (float)frames;
}

float Timer::AsFloat()
{
    return this->currentF;
}
