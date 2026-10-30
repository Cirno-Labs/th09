#include "unsorted.h"

#include "chain.h"
#include "timer.h"

float Sqrt(float x)
{
    return (float)sqrt((double)x);
}

// Outside timer.cpp: there the compiler would know that Initialize leaves
// ecx alone, and return it instead of a copy kept in esi.
Timer::Timer()
{
    this->Initialize();
}

DWORD Rng::GetRandomU32InRange(DWORD range)
{
    return range != 0 ? this->GetRandomU32() % range : 0;
}

float Rng::GetRandomF32InRange(float range)
{
    return this->GetRandomF32ZeroToOne() * range;
}

float Atan2(float y, float x)
{
    return (float)atan2((double)y, (double)x);
}

float Fmod(float x, float y)
{
    return (float)fmod((double)x, (double)y);
}

float Rng::GetRandomF32SignedInRange(float range)
{
    return this->GetRandomF32MinusOneToOne() * range;
}

void Vector2::SetPolarEllipse(float angle, float radiusX, float radiusY)
{
    __asm
    {
        mov eax, this
        fld angle
        fsincos
        fmul radiusX
        fstp [eax]
        fmul radiusY
        fstp [eax+4]
    }
}

float Fabs(float x)
{
    return (float)fabs((double)x);
}

void SinCos(float *sinOut, float *cosOut, float angle)
{
    __asm
    {
        fld angle
        fsincos
        mov eax, cosOut
        fstp [eax]
        mov eax, sinOut
        fstp [eax]
    }
}

ChainElem::ChainElem()
{
    this->prev = NULL;
    this->next = NULL;
    this->callback = NULL;
    this->self = this;
    this->addedCallback = NULL;
    this->deletedCallback = NULL;
    this->priority = 0;
    this->allocated = 0;
}

ChainElem::~ChainElem()
{
    if (this->deletedCallback != NULL)
    {
        this->deletedCallback(this->arg);
    }
    this->prev = NULL;
    this->next = NULL;
    this->callback = NULL;
    this->addedCallback = NULL;
    this->deletedCallback = NULL;
}

Chain::Chain()
{
}

void ResetKeyboard()
{
    BYTE keyState[0x100];
    int i;

    GetKeyboardState(keyState);
    for (i = 0; i < 0x100; i++)
    {
        keyState[i] &= 0x7f;
    }
    SetKeyboardState(keyState);
}

WORD Rng::GetRandomU16()
{
    WORD a;

    a = (this->seed ^ 0x9630) - 0x6553;
    this->seed = (((a & 0xc000) >> 14) + a * 4) & 0xffff;
    this->generated++;
    return this->seed;
}

DWORD Rng::GetRandomU32()
{
    return (this->GetRandomU16() << 16) | this->GetRandomU16();
}

float Rng::GetRandomF32ZeroToOne()
{
    return this->GetRandomU32() / 4294967296.0f;
}

float Rng::GetRandomF32MinusOneToOne()
{
    return this->GetRandomU32() / 2147483648.0f - 1.0f;
}

float AddNormalizeAngle(float a, float b)
{
    int i = 0;

    a += b;
    while (a > 3.1415927f)
    {
        a -= 6.2831855f;
        if (i++ > 16)
        {
            break;
        }
    }
    while (a < -3.1415927f)
    {
        a += 6.2831855f;
        if (i++ > 16)
        {
            break;
        }
    }
    return a;
}

void Rotate2D(D3DXVECTOR3 *out, D3DXVECTOR3 *in, float angle)
{
    float s = sinf(angle);
    float c = cosf(angle);

    out->x = c * in->x - s * in->y;
    out->y = c * in->y + s * in->x;
}

void HermiteInterpolate(D3DXVECTOR3 *out, D3DXVECTOR3 *p1, D3DXVECTOR3 *p0, D3DXVECTOR3 *m1, D3DXVECTOR3 *m0,
                        float time, float duration)
{
    float t;
    float f00;
    float f01;
    float f10;
    float f11;

    t = time / duration;
    f00 = (2.0f * t + 1.0f) * (t - 1.0f) * (t - 1.0f);
    f01 = (3.0f - 2.0f * t) * t * t;
    f10 = (1.0f - t) * (1.0f - t) * t;
    f11 = (t - 1.0f) * t * t;
    out->x = f00 * p0->x + f01 * p1->x + f10 * m0->x + f11 * m1->x;
    out->y = f00 * p0->y + f01 * p1->y + f10 * m0->y + f11 * m1->y;
}

MemoryManager::MemoryManager()
{
    this->freeBlocksOnExit = 0;
}

MemoryManager::~MemoryManager()
{
    int i;

    if (this->freeBlocksOnExit != 0)
    {
        for (i = 0; i < 0x1000; i++)
        {
            if (this->blocks[i] != NULL)
            {
                free(this->blocks[i]);
            }
        }
    }
}

Log::Log()
{
    this->cursor = this->buffer;
    *this->cursor = '\0';
    this->hasError = 0;
}

void App::EnterCriticalSection(int idx)
{
    ::EnterCriticalSection(&this->criticalSections[idx]);
    this->criticalSectionDepths[idx]++;
}

void App::LeaveCriticalSection(int idx)
{
    ::LeaveCriticalSection(&this->criticalSections[idx]);
    this->criticalSectionDepths[idx]--;
}

Chain::~Chain()
{
}

int Chain::AddToCalcChain(ChainElem *elem, int priority)
{
    ChainElem *cur = &this->calcChain;
    int result = 0;

    if (elem->addedCallback != NULL)
    {
        result = elem->addedCallback(elem->arg);
        elem->addedCallback = NULL;
    }
    g_App.EnterCriticalSection(0);
    elem->priority = priority;
    while (cur->next != NULL)
    {
        if (cur->priority > priority)
        {
            break;
        }
        cur = cur->next;
    }
    if (cur->priority > priority)
    {
        elem->next = cur;
        elem->prev = cur->prev;
        if (elem->prev != NULL)
        {
            elem->prev->next = elem;
        }
        cur->prev = elem;
    }
    else
    {
        elem->next = NULL;
        elem->prev = cur;
        cur->next = elem;
    }
    g_App.LeaveCriticalSection(0);
    return result;
}

int Chain::AddToDrawChain(ChainElem *elem, int priority)
{
    ChainElem *cur = &this->drawChain;
    int result = 0;

    if (elem->addedCallback != NULL)
    {
        result = elem->addedCallback(elem->arg);
        elem->addedCallback = NULL;
    }
    g_App.EnterCriticalSection(0);
    elem->priority = priority;
    while (cur->next != NULL)
    {
        if (cur->priority > priority)
        {
            break;
        }
        cur = cur->next;
    }
    if (cur->priority > priority)
    {
        elem->next = cur;
        elem->prev = cur->prev;
        if (elem->prev != NULL)
        {
            elem->prev->next = elem;
        }
        cur->prev = elem;
    }
    else
    {
        elem->next = NULL;
        elem->prev = cur;
        cur->next = elem;
    }
    g_App.LeaveCriticalSection(0);
    return result;
}

ChainElem *Chain::CreateElem(ChainCallback callback)
{
    ChainElem *elem = (ChainElem *)g_MemoryManager.Register(new ChainElem(), sizeof(ChainElem), "funcChainInf");
    elem->SetCallback(callback);
    elem->allocated = 1;
    return elem;
}

void Chain::RemoveLocked(ChainElem *elem)
{
    ChainElem *cur;
    ChainCallback callback;

    if (elem == NULL)
    {
        return;
    }
    cur = &this->calcChain;
    while (cur != NULL)
    {
        if (cur == elem)
        {
            goto found;
        }
        cur = cur->next;
    }
    cur = &this->drawChain;
    while (cur != NULL)
    {
        if (cur == elem)
        {
            goto found;
        }
        cur = cur->next;
    }
    return;

found:
    if (elem->prev != NULL)
    {
        elem->callback = NULL;
        elem->prev->next = elem->next;
        if (elem->next != NULL)
        {
            elem->next->prev = elem->prev;
        }
        elem->prev = NULL;
        elem->next = NULL;
        if (elem->allocated)
        {
            g_App.LeaveCriticalSection(0);
            delete elem;
            elem = NULL;
            g_App.EnterCriticalSection(0);
        }
        else if (elem->deletedCallback != NULL)
        {
            callback = elem->deletedCallback;
            elem->deletedCallback = NULL;
            g_App.LeaveCriticalSection(0);
            callback(elem->arg);
            g_App.EnterCriticalSection(0);
        }
    }
}

int LogPrintf(Log *log, const char *fmt, ...)
{
    int len2;
    char buf[0x2000];
    va_list args;

    va_start(args, fmt);
    g_App.EnterCriticalSection(3);
    vsprintf(buf, fmt, args);
    len2 = strlen(buf);
    if (log->cursor + len2 < log->buffer + sizeof(log->buffer) - 1)
    {
        strcpy(log->cursor, buf);
        log->cursor += len2;
        *log->cursor = '\0';
    }
    va_end(args);
    g_App.LeaveCriticalSection(3);
    return (int)fmt;
}

int LogError(Log *log, const char *fmt, ...)
{
    int len2;
    char buf[0x200];
    va_list args;

    va_start(args, fmt);
    g_App.EnterCriticalSection(3);
    vsprintf(buf, fmt, args);
    len2 = strlen(buf);
    if (log->cursor + len2 < log->buffer + sizeof(log->buffer) - 1)
    {
        strcpy(log->cursor, buf);
        log->cursor += len2;
        *log->cursor = '\0';
    }
    va_end(args);
    log->hasError = 1;
    g_App.LeaveCriticalSection(3);
    return (int)fmt;
}

WORD InitJoystick()
{
    JOYINFOEX joyInfo;

    joyInfo.dwSize = sizeof(joyInfo);
    joyInfo.dwFlags = JOY_RETURNALL;
    if (joyGetPosEx(JOYSTICKID1, &joyInfo) != JOYERR_NOERROR &&
        joyGetPosEx(JOYSTICKID2, &joyInfo) != JOYERR_NOERROR)
    {
        LogPrintf(&g_Log, "使えるパッドが存在しないようです、残念\r\n");
        return 1;
    }
    joyGetDevCapsA(JOYSTICKID1, &g_JoyCaps[0], sizeof(g_JoyCaps[0]));
    joyGetDevCapsA(JOYSTICKID2, &g_JoyCaps[1], sizeof(g_JoyCaps[1]));
    return 0;
}

void Chain::Remove(ChainElem *elem)
{
    g_App.EnterCriticalSection(0);
    this->RemoveLocked(elem);
    g_App.LeaveCriticalSection(0);
}

void Chain::ReleaseList(ChainElem *root)
{
    ChainElem list;
    ChainElem *tail;
    ChainElem *tmp;
    ChainElem *cur;

    tail = (ChainElem *)g_MemoryManager.Register(new ChainElem(), sizeof(ChainElem), "funcChainInf");
    list.next = tail;
    cur = root;
    while (cur != NULL)
    {
        tail->self = cur;
        tail->next = (ChainElem *)g_MemoryManager.Register(new ChainElem(), sizeof(ChainElem), "funcChainInf");
        tail = tail->next;
        cur = cur->next;
    }
    cur = &list;
    while (cur != NULL)
    {
        this->Remove(cur->self);
        cur = cur->next;
    }
    tail = list.next;
    while (tail != NULL)
    {
        tmp = tail->next;
        delete tail;
        tail = NULL;
        tail = tmp;
    }
}

void Chain::Release()
{
    g_App.CloseSubThread();
    this->ReleaseList(&this->calcChain);
    this->ReleaseList(&this->drawChain);
}

float Tan(float angle)
{
    return (float)tan((double)angle);
}

float Atan(float x)
{
    return (float)atan((double)x);
}

void Vector2::SetPolar(float angle, float radius)
{
    __asm
    {
        mov eax, this
        fld angle
        fsincos
        fmul radius
        fstp [eax]
        fmul radius
        fstp [eax+4]
    }
}
