#include "unsorted.h"

float Cos(float angle)
{
    return cosf(angle);
}

float Sin(float angle)
{
    return sinf(angle);
}

float Sqrt(float x)
{
    return sqrtf(x);
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
