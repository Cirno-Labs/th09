// The game's data archive, as th08's.

#pragma once

#include <windows.h>

#include "common.h"

// The archive the game's files are read from, as th08's (at 0x4ad008).
class DatArchive
{
public:
    // The decoded size of entry `name`; 0 if there is none (0x433290).
    DWORD GetEntrySize(char *name);
    // Decodes entry `name` into `buffer`, which must hold GetEntrySize
    // bytes (0x4331c0).
    void *Decode(char *name, void *buffer);
};
extern DatArchive g_Archive;
