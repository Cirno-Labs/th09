// The game's files: the data archive (from 0x433040), and the file
// interface and its Win32 implementation (from 0x4336a0), as th08's.

#pragma once

#include <windows.h>

#include "common.h"

// An abstract file, the interface File implements, as th08's.
class FileBase
{
public:
    virtual bool Open(char *path, char *mode) = 0;
    virtual void Close() = 0;
    virtual DWORD Read(void *buffer, DWORD size) = 0;
    virtual bool Write(void *buffer, DWORD size) = 0;
    virtual DWORD Tell() = 0;
    virtual DWORD GetSize() = 0;
    virtual bool Seek(LONG offset, DWORD origin) = 0;
    // Inline; the copy is at 0x433900.
    virtual ~FileBase()
    {
    }
};

// A Win32 file, as th08's.
class File : public FileBase
{
public:
    // 0x433940.
    File();
    // Calls Close (0x433960).
    ~File();

    // Opens `path`, relative to the game's directory unless it has a
    // drive, in `mode` ("r", "w" or "a"). Returns true on success
    // (0x433980).
    virtual bool Open(char *path, char *mode);
    // 0x4336a0.
    virtual void Close();
    // Returns the bytes read (0x4336d0).
    virtual DWORD Read(void *buffer, DWORD size);
    // Returns true if all `size` bytes were written (0x433710).
    virtual bool Write(void *buffer, DWORD size);
    // 0x433760.
    virtual DWORD Tell();
    // 0x433780.
    virtual DWORD GetSize();
    // 0x4337a0.
    virtual bool Seek(LONG offset, DWORD origin);
    // Reads the whole file, if it is at most `maxSize` bytes, into a
    // GlobalAlloc'd buffer, and seeks back to where it was (0x4337d0).
    virtual void *ReadAll(DWORD maxSize);

    // Reads 4 bytes into `dst`. Returns the bytes read. Inline; the copy
    // is at 0x4330e0.
    DWORD ReadDword(void *dst)
    {
        return this->Read(dst, 4);
    }

    HANDLE handle;
    // GENERIC_READ or GENERIC_WRITE.
    DWORD access;
};
ASSERT_SIZE(File, 0xc);

// FILE_BEGIN, FILE_CURRENT and FILE_END, at 0x490f08. Only the first is
// used.
extern const DWORD g_SeekOrigins[3];
// "r", "w" and "a", the modes for File::Open, at 0x4a1ec8. Only the
// first is used.
extern char *g_FileModes[3];

// Writes `path` to `dst` (MAX_PATH bytes) as a full path: in the game's
// directory unless it has a drive (0x433870).
void MakeFullPath(char *dst, char *path);

// An entry of an archive's table, as th08's. Its constructor and
// destructor are inline; the copies are at 0x433130 and 0x433140.
struct DatArchiveEntry
{
    DatArchiveEntry()
    {
        this->name = NULL;
    }
    ~DatArchiveEntry()
    {
        if (this->name != NULL)
        {
            GlobalFree(this->name);
            this->name = NULL;
        }
    }

    // GlobalAlloc'd.
    char *name;
    // Where the compressed data starts in the archive; it ends where the
    // next entry's starts. The table has one more entry, without a name,
    // whose offset is the table's.
    DWORD offset;
    // The decoded size.
    DWORD size;
    DWORD checksum;
};
ASSERT_SIZE(DatArchiveEntry, 0x10);

// The header of the archive after the magic: three dwords, encrypted
// with DecryptData and each offset by a constant, as th08's.
struct DatArchiveHeader
{
    int numEntries;
    DWORD tableOffset;
    DWORD decodedSize;
};
ASSERT_SIZE(DatArchiveHeader, 0xc);

// Walks the decoded entry table in BuildEntryTable, as th08's. Its
// inline methods are out-of-line copies at 0x433100 and 0x433110.
struct DatArchiveCursor
{
    // Moves past the dword at the cursor and returns the next one.
    DWORD SkipDword()
    {
        this->cur += 4;
        return *(DWORD *)this->cur;
    }
    // Moves past the string at the cursor and its terminator; returns
    // the new position.
    BYTE *SkipString()
    {
        this->cur += strlen((char *)this->cur) + 1;
        return this->cur;
    }

    BYTE *cur;
};

// The archive the game's files are read from (magic "PBGZ"), as th08's,
// at 0x4ad008: files are looked up by name (without the directory).
class DatArchive
{
public:
    // 0x433040.
    DatArchive();
    // Calls Release (0x4335d0).
    ~DatArchive();

    // Opens the archive at `path` and reads its table. Returns true on
    // success (0x4335e0).
    bool Open(char *path);
    // Closes the archive and frees its table (0x4333b0).
    void Release();
    // The decoded size of entry `name`; 0 if there is none (0x433290).
    DWORD GetEntrySize(char *name);
    // Decodes entry `name` into `buffer`, which must hold GetEntrySize
    // bytes (0x4331c0).
    void *Decode(char *name, void *buffer);
    // The entry named `name` (case-insensitive), or NULL (0x433050).
    DatArchiveEntry *FindEntry(char *name);
    // Opens the file at `path` and reads the header and the table, each
    // decrypted with DecryptData. Returns true on success; on failure
    // deletes the file (0x4333f0).
    bool ReadTable(char *path);
    // Builds the entry table from the `count` entries at `table` (each a
    // name, then offset, size and checksum), plus an end entry at
    // `tableOffset`. Returns a new[]'d array, or NULL (0x4332b0).
    DatArchiveEntry *BuildEntryTable(BYTE *table, int count, DWORD tableOffset);
    // A GlobalAlloc'd copy of `str`, or NULL (0x4330a0).
    char *DuplicateString(char *str);

    DatArchiveEntry *entries;
    int numEntries;
    // The archive's path, GlobalAlloc'd.
    char *path;
    File *file;
};
ASSERT_SIZE(DatArchive, 0x10);
extern DatArchive g_Archive;
