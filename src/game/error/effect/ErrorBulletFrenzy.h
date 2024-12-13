//
// Created by Matq on 13/12/2024.
//

#pragma once

#include "../base/ErrorStatusEffect.h"

class ErrorBulletFrenzy : public ErrorStatusEffect {
private:
    static LoadedTexture sTextureBulletFrenzy;

public:
    explicit ErrorBulletFrenzy(ErrorStatuses* parent);

};
