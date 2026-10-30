// A frame counter that can run in fractions of a frame.

#pragma once

#include <windows.h>

#include "common.h"

struct Timer
{
    // Calls Initialize (0x403390).
    Timer();
    // Starts counting from 0, with no previous count (-999999)
    // (0x4014a0).
    void Initialize();
    // Advances by one frame through App::TickTimer and returns the new
    // count (0x4014b0).
    int NextTick();
    // Sets the count to `frames`, with no previous count (0x4014d0).
    void Set(int frames);
    // Advances by `frames`, scaled by gameSpeed while the game runs slow;
    // while bit 0x20 of g_App.stateFlags is set, adds the count to the float
    // once more and forgets the previous count (0x42f490).
    void Add(float frames);
    // Add, from a whole number of frames (0x406600).
    void Add(int frames);
    // Add(-frames) (0x4065e0, 0x406620).
    void Sub(float frames);
    void Sub(int frames);
    // Add and Sub as operators (0x406640, 0x406650, 0x406660, 0x41f300).
    void operator+=(int frames);
    void operator+=(float frames);
    void operator-=(float frames);
    void operator-=(int frames);
    // Sub(1), as a postfix (0x406670).
    void operator--(int);
    // Whether the count changed since the last Tick or Set (0x40f810).
    BOOL HasTicked();
    // The count with its fraction (0x4014f0).
    float AsFloat();
    // Set, as an assignment (0x401500).
    void operator=(int frames);
    // NextTick, as a postfix (0x401510).
    void operator++(int);
    BOOL operator>=(int frames);
    BOOL operator<(int frames);
    BOOL operator>(int frames);
    BOOL operator==(int frames);
    BOOL operator<=(int frames);
    // The count modulo `n` (0x40f830).
    int operator%(int n);

    // `current` before the last change.
    int previous;
    // The count with its fraction; `current` is its integer part.
    float currentF;
    int current;
};
ASSERT_SIZE(Timer, 0xc);
