#include "unsorted.h"

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
