// Classes and globals not yet placed in their own files.

#pragma once

#include <math.h>

#include "common.h"

// cosf, a single x87 instruction under /Oi, as an out-of-line function
// (0x401060). Sin and Sqrt follow it.
float Cos(float angle);
float Sin(float angle);
float Sqrt(float x);

// A 2D vector, as th08's.
struct Vector2
{
    // Sets x and y to `radius` along `angle`, with one fsincos (0x441890).
    void SetPolar(float angle, float radius);

    float x;
    float y;
};
