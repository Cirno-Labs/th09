// The startup entry at 0x48d920 constructs twenty 16-byte archives and
// registers their destructor. No retained game function uses the array.
#include "../archive.h"

DatArchive g_UnusedArchives[20];
