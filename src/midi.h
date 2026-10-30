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
};
