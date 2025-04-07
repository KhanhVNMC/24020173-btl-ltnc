#ifndef TIAF_H
#define TIAF_H
#include "../normal_entity.h"

// available debuff(s)
enum Debuff {
    BLIND,
    NO_HOLD,
    SUPER_SONIC,
    WEAKNESS,
    FRAGILE
};

class DebuffFairy : public NormalEntity {
public:
    DebuffFairy(const void* tetrisPlayer) : NormalEntity(tetrisPlayer) {
        this->setTextureFile("../assets/miniboss_01.bmp"); // blue nigga
        this->setDamageThresholds(1, 4); // 1-4 dmg
        this->setAttackSpeed(5); // 30s between attacks
        this->setDifficulty(MEDIUM); // this is a medium mob
        this->setMaxHealth(999); // 12HP (3 tetris(s) to clear)
        this->defaultFrameSpeed = 10;
    }

    virtual vector<Debuff> availableDebuffs() = 0;

    void attackPlayer(const void *p);
};

#endif //TIAF_H
