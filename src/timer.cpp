#include "timer.h"

#include "unsorted.h"

BOOL Timer::operator>(int frames)
{
    return this->current > frames;
}

BOOL Timer::HasTickedEvery(int n)
{
    return this->current != this->previous && this->current % n == 0;
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

int Timer::operator%(int n)
{
    return this->current % n;
}

void Timer::operator-=(int frames)
{
    this->Sub(frames);
}
