// Observe an unmodified game at App::OnUpdate with the Windows debug API.
// The breakpoint exists only in process memory and is restored for each step.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "player.h"
#include "bullet.h"
#include "enemy.h"
#include "replay.h"

static HANDLE process;

static void Fail(const char *message)
{
    fprintf(stderr, "parity probe: %s (Windows error %lu)\n", message, GetLastError());
    if (process) TerminateProcess(process, 2);
    ExitProcess(2);
}

template <class T> static T Read(DWORD address)
{
    T value;
    SIZE_T count;
    if (!ReadProcessMemory(process, (void *)address, &value, sizeof(value), &count) || count != sizeof(value))
        Fail("cannot read observed game state");
    return value;
}

static void WriteByte(DWORD address, BYTE value)
{
    SIZE_T count;
    if (!WriteProcessMemory(process, (void *)address, &value, 1, &count) || count != 1)
        Fail("cannot update debugger breakpoint");
    if (!FlushInstructionCache(process, (void *)address, 1)) Fail("cannot flush breakpoint");
}

static void Side(FILE *out, DWORD base)
{
    DWORD player = Read<DWORD>(base + offsetof(GameSide, player));
    DWORD stats = Read<DWORD>(base + offsetof(GameSide, stats));
    DWORD bullets = Read<DWORD>(base + offsetof(GameSide, bulletManager));
    DWORD enemies = Read<DWORD>(base + offsetof(GameSide, enemyManager));
    if (!player || !stats || !bullets || !enemies) Fail("replay has incomplete side owners");
    fprintf(out, "{\"character\":%ld,\"state\":%ld,\"health\":%ld,\"x_bits\":%lu,\"y_bits\":%lu,"
                 "\"charge_bits\":%lu,\"score\":%lu,\"lives_bits\":%lu,\"bullets\":%ld,\"enemies\":%ld}",
            Read<LONG>(base + offsetof(GameSide, character)),
            Read<LONG>(player + offsetof(Player, unknown0)),
            Read<LONG>(player + offsetof(Player, unknownA8)),
            Read<DWORD>(player + offsetof(Player, pos)),
            Read<DWORD>(player + offsetof(Player, pos) + 4),
            Read<DWORD>(player + offsetof(Player, unknown30388)),
            Read<DWORD>(stats + offsetof(GameStats, score)),
            Read<DWORD>(stats + offsetof(GameStats, lives)),
            Read<LONG>(bullets + offsetof(BulletManager, bulletCount)),
            Read<LONG>(enemies + offsetof(EnemyManager, enemyCount)));
}

int main(int argc, char **argv)
{
    if (argc != 10)
    {
        fprintf(stderr, "usage: parity_probe GAME TRACE FRAMES TIMEOUT BREAK APP MANAGER RNG INPUTS\n");
        return 2;
    }
    DWORD limit = strtoul(argv[3], NULL, 0);
    DWORD timeout = strtoul(argv[4], NULL, 0) * 1000;
    DWORD breakpoint = strtoul(argv[5], NULL, 0);
    DWORD app = strtoul(argv[6], NULL, 0);
    DWORD manager = strtoul(argv[7], NULL, 0);
    DWORD rng = strtoul(argv[8], NULL, 0);
    DWORD inputs = strtoul(argv[9], NULL, 0);
    FILE *out = fopen(argv[2], "wb");
    if (!out || !limit || !timeout) Fail("invalid output or capture limits");
    STARTUPINFOA startup = {sizeof(startup)};
    PROCESS_INFORMATION child = {0};
    if (!CreateProcessA(argv[1], NULL, NULL, NULL, FALSE, DEBUG_ONLY_THIS_PROCESS,
                        NULL, NULL, &startup, &child)) Fail("cannot launch game");
    process = child.hProcess;
    CloseHandle(child.hThread);
    DWORD started = GetTickCount();
    DWORD samples = 0;
    DWORD stepping = 0;
    DWORD lastFrame = 0;
    BYTE original = 0;
    bool installed = false;
    bool complete = false;
    bool loaderBreakSeen = false;
    while (!complete)
    {
        if (GetTickCount() - started > timeout) Fail("capture timed out before requested replay frames");
        DEBUG_EVENT event;
        if (!WaitForDebugEvent(&event, 1000))
        {
            if (GetLastError() == ERROR_SEM_TIMEOUT) continue;
            Fail("cannot wait for debug event");
        }
        DWORD status = DBG_CONTINUE;
        if (event.dwDebugEventCode == CREATE_PROCESS_DEBUG_EVENT)
        {
            if (event.u.CreateProcessInfo.hFile) CloseHandle(event.u.CreateProcessInfo.hFile);
            original = Read<BYTE>(breakpoint);
            if (Read<DWORD>(breakpoint) != 0xf18b5653) Fail("App::OnUpdate prologue differs");
            WriteByte(breakpoint, 0xcc);
            installed = true;
        }
        else if (event.dwDebugEventCode == LOAD_DLL_DEBUG_EVENT)
        {
            if (event.u.LoadDll.hFile) CloseHandle(event.u.LoadDll.hFile);
        }
        else if (event.dwDebugEventCode == CREATE_THREAD_DEBUG_EVENT)
        {
            CloseHandle(event.u.CreateThread.hThread);
        }
        else if (event.dwDebugEventCode == EXCEPTION_DEBUG_EVENT)
        {
            EXCEPTION_DEBUG_INFO *exception = &event.u.Exception;
            DWORD code = exception->ExceptionRecord.ExceptionCode;
            DWORD address = (DWORD)exception->ExceptionRecord.ExceptionAddress;
            if (installed && code == EXCEPTION_BREAKPOINT && address == breakpoint)
            {
                HANDLE thread = OpenThread(THREAD_GET_CONTEXT | THREAD_SET_CONTEXT, FALSE, event.dwThreadId);
                if (!thread) Fail("cannot open observed thread");
                CONTEXT context = {0};
                context.ContextFlags = CONTEXT_CONTROL;
                if (!GetThreadContext(thread, &context)) Fail("cannot read breakpoint context");
                context.Eip = breakpoint;
                context.EFlags |= 0x100;
                WriteByte(breakpoint, original);
                if (!SetThreadContext(thread, &context)) Fail("cannot step original instruction");
                CloseHandle(thread);
                stepping = event.dwThreadId;
                DWORD replay = Read<DWORD>(manager + offsetof(GameManager, replayManager));
                if (Read<LONG>(app + offsetof(App, scene)) == 2 &&
                    !Read<LONG>(manager + offsetof(GameManager, loadingThreadState)) &&
                    (Read<DWORD>(manager + offsetof(GameManager, stateFlags)) & 8) && replay &&
                    Read<LONG>(replay + offsetof(ReplayManager, mode)) == 1)
                {
                    DWORD frame = Read<DWORD>(manager + offsetof(GameManager, frames));
                    if (!samples && frame != 0) Fail("missed the first replay frame");
                    if (!samples || frame != lastFrame)
                    {
                        if (samples && frame != lastFrame + 1) Fail("replay frame sequence jumped or reset");
                        fprintf(out, "{\"frame\":%lu,\"app_frame\":%lu,\"rng_seed\":%u,\"rng_generated\":%lu,"
                                     "\"stage\":%ld,\"difficulty\":%ld,\"damage\":%ld,\"cpu_damage\":%ld,"
                                     "\"input0\":%lu,\"input1\":%lu,\"sides\":[",
                                frame, Read<DWORD>(app + offsetof(App, frames)),
                                Read<WORD>(rng + offsetof(Rng, seed)), Read<DWORD>(rng + offsetof(Rng, generated)),
                                Read<LONG>(manager + offsetof(GameManager, stage)),
                                Read<LONG>(manager + offsetof(GameManager, difficulty)),
                                Read<LONG>(manager + offsetof(GameManager, rank) + offsetof(GameRank, unknownB8)),
                                Read<LONG>(manager + offsetof(GameManager, rank) + offsetof(GameRank, unknownBc)),
                                (DWORD)Read<WORD>(inputs + offsetof(PlayerInput, unknown2c)),
                                (DWORD)Read<WORD>(inputs + sizeof(PlayerInput) + offsetof(PlayerInput, unknown2c)));
                        Side(out, manager + offsetof(GameManager, sides));
                        fputc(',', out);
                        Side(out, manager + offsetof(GameManager, sides) + sizeof(GameSide));
                        fputs("]}\n", out);
                        fflush(out);
                        lastFrame = frame;
                        if (++samples == limit) complete = true;
                    }
                }
            }
            else if (code == EXCEPTION_SINGLE_STEP && stepping == event.dwThreadId)
            {
                HANDLE thread = OpenThread(THREAD_GET_CONTEXT | THREAD_SET_CONTEXT, FALSE, event.dwThreadId);
                if (!thread) Fail("cannot open stepped thread");
                CONTEXT context = {0};
                context.ContextFlags = CONTEXT_CONTROL;
                if (!GetThreadContext(thread, &context)) Fail("cannot read stepped context");
                context.EFlags &= ~0x100;
                if (!SetThreadContext(thread, &context)) Fail("cannot clear single-step flag");
                CloseHandle(thread);
                WriteByte(breakpoint, 0xcc);
                stepping = 0;
            }
            else if (code == EXCEPTION_BREAKPOINT && !loaderBreakSeen && exception->dwFirstChance)
            {
                loaderBreakSeen = true;
            }
            else
            {
                if (!exception->dwFirstChance)
                {
                    fprintf(stderr, "parity probe: unhandled exception %08lx at %08lx\n", code, address);
                    Fail("game crashed");
                }
                status = DBG_EXCEPTION_NOT_HANDLED;
            }
        }
        else if (event.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT)
        {
            fprintf(stderr, "parity probe: game exited (%lu), captured %lu/%lu frames\n",
                    event.u.ExitProcess.dwExitCode, samples, limit);
            Fail("incomplete replay capture");
        }
        if (!ContinueDebugEvent(event.dwProcessId, event.dwThreadId, status)) Fail("cannot continue game");
    }
    fclose(out);
    TerminateProcess(process, 0);
    CloseHandle(process);
    printf("captured %lu replay frames\n", samples);
    return 0;
}
