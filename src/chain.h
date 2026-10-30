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

    // Sets the callback and clears the added and deleted callbacks. An
    // inline function in th08; the copy the game calls is in an object
    // built with /O1 (0x41a889).
    void SetCallback(ChainCallback callback);

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
    // Points to the element itself; in ReleaseList's copy of a list, to
    // the element copied.
    ChainElem *self;
    void *arg;
};
ASSERT_SIZE(ChainElem, 0x20);

// The heads of the update and draw chains, at 0x4acfc8.
class Chain
{
public:
    // Constructs the two heads (0x42ac30).
    Chain();
    // Destroys the two heads (0x42b190).
    ~Chain();

    // Calls `elem`'s added callback, clearing it, then inserts `elem`
    // into the update chain by priority, inside critical section 0.
    // Returns the callback's result, or 0 without one (0x42b1e0).
    int AddToCalcChain(ChainElem *elem, int priority);
    // Like AddToCalcChain, for the draw chain (0x42b270).
    int AddToDrawChain(ChainElem *elem, int priority);
    // Allocates an element that runs `callback` (0x42b300).
    ChainElem *CreateElem(ChainCallback callback);
    // Unlinks `elem` from its chain, for a caller that holds critical
    // section 0, which it leaves while the element is deleted or its
    // deleted callback runs (0x42b350).
    void RemoveLocked(ChainElem *elem);
    // Like RemoveLocked, inside critical section 0 (0x42c8c0).
    void Remove(ChainElem *elem);
    // Removes the elements of the list that starts at `root`, going
    // through a copy of the list it allocates (0x42cae0).
    void ReleaseList(ChainElem *root);

    ChainElem calcChain;
    ChainElem drawChain;
};
ASSERT_SIZE(Chain, 0x40);
extern Chain g_Chain;
