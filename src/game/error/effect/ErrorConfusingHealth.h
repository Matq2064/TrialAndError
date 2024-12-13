//
// Created by Matq on 13/12/2024.
//

#pragma once

#include "../base/ErrorStatusEffect.h"

class ErrorConfusingHealth : public ErrorStatusEffect {
private:
    static LoadedTexture sTextureConfusingHealth;

public:
    explicit ErrorConfusingHealth(ErrorStatuses* parent);

};
