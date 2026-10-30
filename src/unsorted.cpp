#include "unsorted.h"

#include "timer.h"

float Cos(float angle)
{
    return (float)cos((double)angle);
}

float Sin(float angle)
{
    return (float)sin((double)angle);
}

float Sqrt(float x)
{
    return (float)sqrt((double)x);
}

// Outside timer.cpp: there the compiler would know that Initialize leaves
// ecx alone, and return it instead of a copy kept in esi.
Timer::Timer()
{
    this->Initialize();
}

float Atan2(float y, float x)
{
    return (float)atan2((double)y, (double)x);
}

float Fmod(float x, float y)
{
    return (float)fmod((double)x, (double)y);
}

float Fabs(float x)
{
    return (float)fabs((double)x);
}

void Vector2::SetPolar(float angle, float radius)
{
    __asm
    {
        mov eax, this
        fld angle
        fsincos
        fmul radius
        fstp [eax]
        fmul radius
        fstp [eax+4]
    }
}
