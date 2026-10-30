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

WORD Rng::GetRandomU16()
{
    WORD a;

    a = (this->seed ^ 0x9630) - 0x6553;
    this->seed = (((a & 0xc000) >> 14) + a * 4) & 0xffff;
    this->generated++;
    return this->seed;
}

DWORD Rng::GetRandomU32()
{
    return (this->GetRandomU16() << 16) | this->GetRandomU16();
}

float Tan(float angle)
{
    return (float)tan((double)angle);
}

float Atan(float x)
{
    return (float)atan((double)x);
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
