// Classes and globals not yet placed in their own files.

#pragma once

#include <math.h>

#include "common.h"

// cosf, a single x87 instruction under /Oi, as an out-of-line function
// (0x401060). Sin and Sqrt follow it.
float Cos(float angle);
float Sin(float angle);
float Sqrt(float x);
