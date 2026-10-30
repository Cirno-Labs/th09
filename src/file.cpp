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

bool File::Write(void *buffer, DWORD size)
{
    DWORD written = 0;

    if (this->access != GENERIC_WRITE)
    {
        return false;
    }
    WriteFile(this->handle, buffer, size, &written, NULL);
    return size == written ? true : false;
}

DWORD File::Tell()
{
    if (this->handle == INVALID_HANDLE_VALUE)
    {
        return 0;
    }
    return SetFilePointer(this->handle, 0, NULL, FILE_CURRENT);
}

DWORD File::GetSize()
{
    if (this->handle == INVALID_HANDLE_VALUE)
    {
        return 0;
    }
    return GetFileSize(this->handle, NULL);
}

bool File::Seek(LONG offset, DWORD origin)
{
    if (this->handle == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    SetFilePointer(this->handle, offset, NULL, origin);
    return true;
}

void *File::ReadAll(DWORD maxSize)
{
    DWORD size;
    HGLOBAL hMem;
    DWORD pos;

    if (this->access != GENERIC_READ)
    {
        return NULL;
    }
    size = this->GetSize();
    if (size > maxSize)
    {
        return NULL;
    }
    hMem = GlobalAlloc(GPTR, size);
    if (hMem == NULL)
    {
        return NULL;
    }
    pos = this->Tell();
    if (!this->Seek(pos, g_SeekOrigins[0]))
    {
        return NULL;
    }
    if (this->Read(hMem, size) == 0)
    {
        if (hMem != NULL)
        {
            GlobalFree(hMem);
            hMem = NULL;
        }
        return NULL;
    }
    this->Seek(pos, g_SeekOrigins[0]);
    return hMem;
}

void MakeFullPath(char *dst, char *path)
{
    char *slash;

    if (strchr(path, ':') != NULL)
    {
        strcpy(dst, path);
    }
    else
    {
        GetModuleFileNameA(NULL, dst, MAX_PATH);
        slash = strrchr(dst, '\\');
        if (slash == NULL)
        {
            strcpy(dst, "");
        }
        slash[1] = '\0';
        strcat(dst, path);
    }
}

File::File()
{
    this->handle = INVALID_HANDLE_VALUE;
    this->access = 0;
}
