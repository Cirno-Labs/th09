#include "screeneffect.h"

void ScreenEffect::Finish()
{
    this->finishing = 1;
    this->timer = 0;
}
