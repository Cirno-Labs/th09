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

void Timer::operator=(int frames)
{
    this->Set(frames);
}

void Timer::operator++(int)
{
    this->NextTick();
}

BOOL Timer::operator>=(int frames)
{
    return this->current >= frames;
}

BOOL Timer::operator<(int frames)
{
    return this->current < frames;
}

BOOL Timer::operator>(int frames)
{
    return this->current > frames;
}

BOOL Timer::operator==(int frames)
{
    return this->current == frames;
}

void Timer::Sub(float frames)
{
    this->Add(-frames);
}

void Timer::Add(int frames)
{
    this->Add((float)frames);
}

void Timer::Sub(int frames)
{
    this->Add(-frames);
}

void Timer::operator+=(int frames)
{
    this->Add(frames);
}

void Timer::operator+=(float frames)
{
    this->Add(frames);
}

void Timer::operator-=(float frames)
{
    this->Sub(frames);
}

void Timer::operator--(int)
{
    this->Sub(1);
}

BOOL Timer::operator<=(int frames)
{
    return this->current <= frames;
}

BOOL Timer::HasTicked()
{
    return this->current != this->previous;
}
