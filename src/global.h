// The exe's precompiled header (exe.pch), as the original's
// ./system/global.h, the __FILE__ its allocations name (GLOBAL_H). Its
// inline functions go into each object's code as the compiler works
// through them, interleaved with the object's own functions, so the
// order of these headers decides where they land
// (docs/natural-link.md).

#pragma once

// For window.cpp's CoInitializeEx.
#define _WIN32_DCOM

#include "unsorted.h"
#include "timer.h"
#include "sprite.h"
#include "gamemanager.h"
#include "stage.h"
#include "ascii.h"
#include "player.h"
#include "archive.h"
#include "bullet.h"
#include "chain.h"
#include "effect.h"
#include "ending.h"
#include "enemy.h"
#include "exattack.h"
#include "gdisurface.h"
#include "gui.h"
#include "lzss.h"
#include "mainmenu.h"
#include "midi.h"
#include "network.h"
#include "replay.h"
#include "scoredat.h"
#include "screeneffect.h"
#include "spellcard.h"
