// The chains that run the game, as th08's: every frame, one runs the
// update callbacks of its elements and the other their draw callbacks,
// in order of priority.

#pragma once

#include <windows.h>

#include "common.h"

// Chain callbacks take the element's `arg`. They return a status; 1
// keeps the element and moves on to the next. With /Gr they are
// __fastcall, taking `arg` in ecx.
typedef int (*ChainCallback)(void *arg);

// An element of a chain.
struct ChainElem
{
    // (0x42abe0)
    ChainElem();
    // Calls the deleted callback, if any, and clears the links and
    // callbacks (0x42ac00).
    ~ChainElem();

    // Lower runs first.
    short priority;
    // Set by Chain::CreateElem: the element was allocated with new.
    unsigned short allocated : 1;
    ChainCallback callback;
    // Called when the element is added to a chain, which clears it
    // after the call.
    ChainCallback addedCallback;
    ChainCallback deletedCallback;
    ChainElem *prev;
    ChainElem *next;
    // Points to the element itself.
    ChainElem *self;
    void *arg;
};
ASSERT_SIZE(ChainElem, 0x20);

// The heads of the update and draw chains, at 0x4acfc8.
class Chain
{
public:
    ChainElem calcChain;
    ChainElem drawChain;
};
ASSERT_SIZE(Chain, 0x40);
extern Chain g_Chain;
