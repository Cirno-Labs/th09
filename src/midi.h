// The MIDI player, as th08's.

#pragma once

#include <windows.h>

#include "common.h"

// Plays MIDI files, as th08's MidiPlayer.
class MidiPlayer
{
public:
    // Loads the file at `path` into `slot` (0x43e0d0).
    int LoadFile(int slot, char *path);
    // Plays the file parsed last; returns -1 without one, else 0
    // (0x43d750).
    int Play();
    // Stops playing; returns -1 without a file, else 0 (0x43de30).
    int Stop();
};
