//
// Created by GiaKhanhVN on 4/7/2025.
//


#ifndef WEAKENERFAIRY_H
#define WEAKENERFAIRY_H
#include "debuff_fairy.h"

class WeakenerFairy : public DebuffFairy {
public:
    WeakenerFairy(const void* tetrisPlayer) : DebuffFairy(tetrisPlayer) {
        this->setTextureFile("../assets/f_weakener.bmp"); // blue nigga
        this->setDamageThresholds(8, 20); // 13-25s debuff blind

        this->setAttackSpeed(30); // 30s between attacks
        this->setDifficulty(MEDIUM); // this is a hard mini boss
        this->setMaxHealth(18); // 14HP

        this->availableDebuffs = { WEAKNESS, FRAGILE };
    }
};

#endif //WEAKENERFAIRY_H