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
    vector<Debuff> availableDebuffs;
    DebuffFairy(TetrisPlayer* tetrisPlayer) : NormalEntity(tetrisPlayer) {
        this->isMiniboss = true;
        this->defaultFrameSpeed = 10;
        this->availableDebuffs = { WEAKNESS, FRAGILE };
    }

    void attackPlayer();
};

#endif //TIAF_H
