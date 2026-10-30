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

void *DatArchive::Decode(char *name, void *buffer)
{
    DatArchiveEntry *found;
    DWORD decodedSize;
    void *decoded;
    HGLOBAL packed;
    DWORD packedSize;

    packed = NULL;
    if (this->file == NULL)
    {
        return NULL;
    }
    found = this->FindEntry(name);
    if (found == NULL)
    {
        goto fail;
    }
    if (!this->file->Open(this->path, g_FileModes[0]))
    {
        goto fail;
    }
    packedSize = found[1].offset - found->offset;
    decodedSize = found->size;
    packed = GlobalAlloc(GMEM_FIXED, packedSize);
    if (packed == NULL)
    {
        goto fail;
    }
    if (!this->file->Seek(found->offset, g_SeekOrigins[0]))
    {
        goto fail;
    }
    if (this->file->Read(packed, packedSize) == 0)
    {
        goto fail;
    }
    decoded = LzssDecompress((BYTE *)packed, packedSize, (BYTE *)buffer, decodedSize);
    if (packed != NULL)
    {
        GlobalFree(packed);
        packed = NULL;
    }
    return decoded;

fail:
    if (packed != NULL)
    {
        GlobalFree(packed);
        packed = NULL;
    }
    return NULL;
}
