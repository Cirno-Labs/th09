// A frame counter that can run in fractions of a frame.

#pragma once

#include <windows.h>

#include "common.h"

struct Timer
{
    // Starts counting from 0, with no previous count (-999999)
    // (0x4014a0).
    void Initialize();
    // Advances by one frame through App::TickTimer and returns the new
    // count (0x4014b0).
    int NextTick();
    // Sets the count to `frames`, with no previous count (0x4014d0).
    void Set(int frames);
    // The count with its fraction (0x4014f0).
    float AsFloat();
    // Set, as an assignment (0x401500).
    void operator=(int frames);
    // NextTick, as a postfix (0x401510).
    void operator++(int);
    BOOL operator>=(int frames);
    BOOL operator<(int frames);

    // `current` before the last change.
    int previous;
    // The count with its fraction; `current` is its integer part.
    float currentF;
    int current;
};
ASSERT_SIZE(Timer, 0xc);
