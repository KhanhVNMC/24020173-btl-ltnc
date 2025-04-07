//
// Created by GiaKhanhVN on 4/7/2025.
//


#ifndef DISTRACTORFAIRY_H
#define DISTRACTORFAIRY_H
#include "debuff_fairy.h"

class DistractorFairy : public DebuffFairy {
public:
    DistractorFairy(const void* tetrisPlayer) : DebuffFairy(tetrisPlayer) {
        this->setTextureFile("../assets/f_distractor.bmp"); // blue nigga
        this->setDamageThresholds(10, 20); // 13-25s debuff blind

        this->setAttackSpeed(30); // 30s between attacks
        this->setDifficulty(MEDIUM); // this is a hard mini boss
        this->setMaxHealth(14); // 14HP

        this->availableDebuffs = { SUPER_SONIC };
    }
};

#endif //DISTRACTORFAIRY_H