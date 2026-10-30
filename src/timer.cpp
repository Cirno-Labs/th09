#include "timer.h"

#include "unsorted.h"

void Timer::Initialize()
{
    this->current = 0;
    this->previous = -999999;
    this->currentF = 0.0f;
}
