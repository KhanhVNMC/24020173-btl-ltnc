//
// Created by GiaKhanhVN on 4/7/2025.
//

#ifndef BLINDERFAIRY_H
#define BLINDERFAIRY_H
#include "debuff_fairy.h"

class BlinderFairy : public DebuffFairy {
public:
    BlinderFairy(const void* tetrisPlayer) : DebuffFairy(tetrisPlayer) {
        this->setTextureFile("../assets/f_blinder.bmp"); // blue nigga
        this->setDamageThresholds(8, 15); // 8-15s debuff blind

        this->setAttackSpeed(25); // 30s between attacks
        this->setDifficulty(HARD); // this is a hard mini boss
        this->setMaxHealth(10); // 12HP (3 tetris(s) to clear)

        this->availableDebuffs = { BLIND };
    }
};

#endif //BLINDERFAIRY_H
