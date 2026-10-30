#include "mainmenu.h"

HRESULT WINAPI NetworkMessageHandler(PVOID context, DWORD type, PVOID message)
{
    return g_Network->HandleMessage(context, type, message);
}
