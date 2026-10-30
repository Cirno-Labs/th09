#include "sprite.h"

void AnmLoaded::StartScriptAtOrigin(Sprite *vm, int script)
{
    vm->scriptIndex = script;
    vm->pos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    vm->posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    vm->fontHeight = 15;
    vm->fontWidth = 15;
    this->SetScript(vm, this->scripts[script]);
}
