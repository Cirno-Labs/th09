// The data archive, from 0x433040.

#include "archive.h"
#include "unsorted.h"

DatArchive::DatArchive()
{
    this->entries = NULL;
    this->numEntries = 0;
    this->path = NULL;
    this->file = NULL;
}

DatArchiveEntry *DatArchive::FindEntry(char *name)
{
    DatArchiveEntry *cur;
    int left;

    if (this->entries == NULL)
    {
        return NULL;
    }
    for (cur = this->entries, left = this->numEntries; left > 0; left--, cur++)
    {
        if (_stricmp(name, cur->name) == 0)
        {
            return cur;
        }
    }
    return NULL;
}

char *DatArchive::DuplicateString(char *str)
{
    char *copy;

    copy = (char *)GlobalAlloc(GMEM_FIXED, strlen(str) + 1);
    if (copy != NULL)
    {
        strcpy(copy, str);
    }
    return copy;
}
