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
