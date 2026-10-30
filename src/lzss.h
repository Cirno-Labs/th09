// The LZSS coder of the data archives, as th08's: a 0x2000-byte window,
// 13-bit offsets and 4-bit lengths (3 to 18 bytes), matches found
// through a binary search tree over the window.

#pragma once

#include <windows.h>

#include "common.h"

// A node of the search tree: one per window position, and a root at
// 0x2000. Links are window positions, 0 for none.
struct LzssNode
{
    int parent;
    int smallerChild;
    int largerChild;
};
ASSERT_SIZE(LzssNode, 0xc);

// The tree; its root is g_LzssTree[0x2000] (0x4b4420).
extern LzssNode g_LzssTree[0x2001];
// The last 0x2000 bytes coded (0x4cc430).
extern BYTE g_LzssWindow[0x2000];

// Decompresses `dataSize` bytes of `data` into `out`, or a new
// GlobalAlloc'd block if NULL, of `outSize` bytes, as th08's
// (0x433aa0). Returns it, or NULL if it cannot be allocated.
void *LzssDecompress(BYTE *data, int dataSize, BYTE *out, int outSize);
// Makes window position `pos` the tree's only node (0x433c80).
void LzssTreeInitNode(int pos);
// Clears the window and the tree (0x433cb0).
void LzssClearWindow();
// Removes node `oldNode`, which has at most one child, `newNode`, from
// the tree, linking the child to its parent (0x433ce0).
void LzssReplaceNode(int oldNode, int newNode);
// Puts node `newNode` in the place of `oldNode` in the tree (0x433d40).
void LzssReplaceChild(int oldNode, int newNode);
// The node that comes before `node`: the largest in its smaller subtree
// (0x433dc0).
int LzssPredecessor(int node);
// Adds the string at window position `pos` to the tree. Returns the
// length of the longest match found and stores its position in
// `matchPosition` (0x433e00).
int LzssInsertNode(int pos, int *matchPosition);
// Removes the string at window position `pos` from the tree (0x433fc0).
void LzssDeleteNode(int pos);
// Compresses `size` bytes at `data` into a new GlobalAlloc block, which
// it returns, and stores the compressed size in `outSize` (0x434020).
void *LzssCompress(BYTE *data, int size, int *outSize);
