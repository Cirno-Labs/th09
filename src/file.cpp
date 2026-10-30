// The file class, from 0x4336a0.

#include "archive.h"

void File::Close()
{
    if (this->handle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(this->handle);
        this->handle = INVALID_HANDLE_VALUE;
        this->access = 0;
    }
}

DWORD File::Read(void *buffer, DWORD size)
{
    DWORD read = 0;

    if (this->access != GENERIC_READ)
    {
        return 0;
    }
    ReadFile(this->handle, buffer, size, &read, NULL);
    return read;
}
