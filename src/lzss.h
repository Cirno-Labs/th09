// The LZSS decoder of the data archives, as th08's: a 0x2000-byte
// window, 13-bit offsets and 4-bit lengths (3 to 18 bytes).

#pragma once

#include <windows.h>

#include "common.h"

// The last 0x2000 bytes decoded (0x4cc430).
extern BYTE g_LzssWindow[0x2000];

// Decompresses `dataSize` bytes of `data` into `out`, or a new
// GlobalAlloc'd block if NULL, of `outSize` bytes, as th08's
// (0x433aa0). Returns it, or NULL if it cannot be allocated.
void *LzssDecompress(BYTE *data, int dataSize, BYTE *out, int outSize);
