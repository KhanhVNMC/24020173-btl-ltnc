//
// Created by GiaKhanhVN on 4/7/2025.
//

#ifndef BLINDERFAIRY_H
#define BLINDERFAIRY_H
#include "debuff_fairy.h"

class BlinderFairy : public DebuffFairy {
public:
    BlinderFairy(TetrisPlayer* tetrisPlayer) : DebuffFairy(tetrisPlayer) {
        this->setTextureFile(FAIRY_BLINDER_SHEET); // blue nigga
        this->setDamageThresholds(10, 20); // 8-15s debuff blind

        this->setAttackSpeed(28); // 30s between attacks
        this->setDifficulty(HARD); // this is a hard mini boss
        this->setMaxHealth(12); // 12HP (3 tetris(s) to clear)

        this->availableDebuffs = { BLIND };
    }
};

#endif //BLINDERFAIRY_H
