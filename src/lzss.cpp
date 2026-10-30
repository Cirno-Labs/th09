// The LZSS decoder, from 0x433aa0, as th08's lzss.cpp without the coder.

#include "lzss.h"

// The next input byte, or 0 past the end, into currentByte.
#define FETCH_BYTE() \
    { \
        currentByte = *in; \
        if (in - data >= size) \
        { \
            currentByte = 0; \
        } \
        else \
        { \
            in++; \
        } \
        checksum += currentByte; \
    }

// The next input bit.
#define READ_BIT(bit) \
    { \
        if (mask == 0x80) \
        { \
            FETCH_BYTE(); \
        } \
        bit = currentByte & mask; \
        mask >>= 1; \
        if (mask == 0) \
        { \
            mask = 0x80; \
        } \
    }

// The next `count` input bits, most significant first. Needs a DWORD
// `bitMask`.
#define READ_BITS(value, count) \
    { \
        bitMask = 1 << ((count) - 1); \
        value = 0; \
        while (bitMask != 0) \
        { \
            if (mask == 0x80) \
            { \
                FETCH_BYTE(); \
            } \
            if (currentByte & mask) \
            { \
                value |= bitMask; \
            } \
            bitMask >>= 1; \
            mask >>= 1; \
            if (mask == 0) \
            { \
                mask = 0x80; \
            } \
        } \
    }

void *LzssDecompress(BYTE *data, int dataSize, BYTE *out, int outSize)
{
    int wndPos;
    DWORD bitMask;
    int ch;
    // The sum of the input bytes; unused.
    DWORD checksum;
    int len;
    int size;
    DWORD bits;
    BYTE *in;
    BYTE mask;
    int i;
    // The position of a match.
    int position;
    BYTE *dst;
    DWORD currentByte;

    mask = 0x80;
    currentByte = 0;
    checksum = 0;
    size = dataSize;
    if (out == NULL)
    {
        out = (BYTE *)GlobalAlloc(GMEM_FIXED, outSize);
        if (out == NULL)
        {
            return NULL;
        }
    }
    in = data;
    dst = out;
    wndPos = 1;
    for (;;)
    {
        READ_BIT(bits);
        if (bits)
        {
            READ_BITS(bits, 8);
            *dst = bits;
            dst++;
            g_LzssWindow[wndPos] = bits;
            wndPos = (wndPos + 1) & 0x1fff;
        }
        else
        {
            READ_BITS(bits, 13);
            position = bits;
            if (position == 0)
            {
                break;
            }
            READ_BITS(bits, 4);
            len = bits + 2;
            for (i = 0; i <= len; i++)
            {
                ch = g_LzssWindow[(position + i) & 0x1fff];
                *dst = ch;
                dst++;
                g_LzssWindow[wndPos] = ch;
                wndPos = (wndPos + 1) & 0x1fff;
            }
        }
    }
    while (mask != 0x80)
    {
        READ_BIT(bits);
    }
    return out;
}
